#pragma once
#include "lib/core/wifi_manager/wifi_manager.h"

namespace module::beacon {
    struct __attribute__((packed)) beacon_frame_t {
        uint8_t frame_ctrl[2] = {0x80, 0x00};
        uint8_t duration[2]   = {0x00, 0x00};
        uint8_t da[6]         = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        uint8_t sa[6]         = {};
        uint8_t bssid[6]      = {};
        uint8_t seq[2]        = {0x00, 0x00};
        // Frame Body
        uint8_t timestamp[8]  = {}; 
        uint8_t interval[2]   = {0x64, 0x00}; // 100 TU = 102.4 ms
        uint8_t capability[2] = {0x31, 0x04}; // ESS | ShortPreamble | WPA
    };
    void send_frame(const uint8_t* bssid, const char* ssid, uint8_t channel, wifi_auth_mode_t authmode);
}