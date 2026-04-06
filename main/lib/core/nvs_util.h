#pragma once

#include "esp_log.h"
#include "nvs_flash.h"
#include <mutex>

namespace core {

inline void ensure_nvs_init() {
    static std::once_flag s_nvs_flag;
    std::call_once(s_nvs_flag, []() {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            nvs_flash_erase();
            nvs_flash_init();
        }
        ESP_LOGI("nvs", "NVS initialized");
    });
}

}
