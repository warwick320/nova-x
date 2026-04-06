#pragma once

#include "esp_wifi_types_generic.h"
#define MAX_DEAUTH_PER_PKT 15

#define ATTACK_TYPE_24GHZ 0
#define ATTACK_TYPE_5GHZ 1
#define ATTACK_TYPE_ALL 2

namespace core {
    enum class wifi_attack_type : uint8_t {
        deauth = 0,
        beacon,
    };
}

#include "esp_err.h"
#include "globals.h"
#include "lib/core/framework.h"
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <vector>

//bypassing deatuh frame sanity check
extern "C" {
    __attribute__((used))
    int ieee80211_raw_frame_sanity_check(void* frame_ctrl, int32_t frame_len, int32_t ampdu_flag) {
        return 0;
    }

    __attribute__((used))
    bool ieee80211_is_frame_type_supported(uint8_t frame_type) {
        return true;
    }

    __attribute__((used))
    int ieee80211_handle_frame_type(void* frame_ctrl, uint32_t* dport) {
        return 0;
    } 
}

namespace core{

    extern wifi_config_t ap_config;

    struct ap_t{
        std::array<uint8_t,6> bssid;
        std::string ssid;
        wifi_auth_mode_t authmode;
        bool act;

        ap_t(const uint8_t* mac, const char* name,
             wifi_auth_mode_t auth = WIFI_AUTH_OPEN)
            : ssid(name), authmode(auth), act(false) {
            memcpy(bssid.data(), mac, 6);
        }

        bool operator==(const ap_t& other) const {
            return bssid == other.bssid;
        }
    };

    class wifi_ctrl{
        private:
            //static std::unique_ptr<wifi_ctrl> instance_;

            wifi_ctrl(void);

            std::atomic<bool> running = false;

            std::map<uint8_t,std::vector<ap_t>> channel_ap_map_2ghz;
            std::map<uint8_t,std::vector<ap_t>> channel_ap_map_5ghz;

            std::map<uint8_t,std::vector<ap_t>> selected_ap_map;

            uint16_t total_ap_num = 0;

            bool scan_status = false;
        public:
            wifi_ctrl(const wifi_ctrl&) = delete;
            wifi_ctrl& operator=(const wifi_ctrl&) = delete;
            wifi_ctrl(wifi_ctrl&&) = delete;
            wifi_ctrl& operator=(wifi_ctrl&&) = delete;
            ~wifi_ctrl();

            [[nodiscard]] static wifi_ctrl& instance(void);

            // [[nodiscard]]
            bool scan(void);
            void select_band_map(std::map<uint8_t,std::vector<ap_t>>& out_map);
            void disable_wdt(bool enabled = false);

            uint8_t attack_band_type = ATTACK_TYPE_ALL;
            uint8_t beacon_band_type = ATTACK_TYPE_ALL;

            [[nodiscard]] bool get_status(void) const { return running.load(); };
            [[nodiscard]] bool get_scan_status(void) const { return scan_status; };
            
            [[nodiscard]] bool scan_status_checker(void) const;

            [[nodiscard]] const std::map<uint8_t,std::vector<ap_t>>& get_channel_ap_map_2ghz(void) const { return channel_ap_map_2ghz; };
            [[nodiscard]] const std::map<uint8_t,std::vector<ap_t>>& get_channel_ap_map_5ghz(void) const { return channel_ap_map_5ghz; };

            [[nodiscard]] uint16_t get_total_ap_num(void) const { return total_ap_num; };

            void attack_start(wifi_attack_type type);
            void attack_stop();
            [[nodiscard]] bool get_attack_status(void) const;
            
    };
}
