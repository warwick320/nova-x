#pragma once
#include "lib/core/wifi_manager/wifi_manager.h"

namespace module::deauth {
    struct __attribute__((packed)) deauth_frame_t{
        uint8_t frame_ctrl[2] = {0xc0,0x00};
        uint8_t duration[2] = {0x3a,0x01};
        uint8_t da[6];
        uint8_t sa[6];
        uint8_t bssid[6];
        uint8_t seq[2] = {0x00,0x00};
        uint8_t reason[2];
    };
    void send_frame(const uint8_t* bssid, const uint8_t* target_mac, uint16_t reason);
}