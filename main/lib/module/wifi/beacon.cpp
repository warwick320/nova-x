#include "beacon.h"
#include "esp_wifi.h"
#include <cstring>
#include <vector>

static const char* TAG = "beacon";

static void append_security_ies(std::vector<uint8_t>& frame, wifi_auth_mode_t auth) {
    static const uint8_t wpa_ie[] = {
        0xDD, 0x18,
        0x00, 0x50, 0xF2, 0x01,
        0x01, 0x00,
        0x00, 0x50, 0xF2, 0x02,
        0x01, 0x00,
        0x00, 0x50, 0xF2, 0x04,
        0x01, 0x00,
        0x00, 0x50, 0xF2, 0x02,
    };
    static const uint8_t rsn_ie_wpa2[] = {
        0x30, 0x14,
        0x01, 0x00,
        0x00, 0x0F, 0xAC, 0x04,
        0x01, 0x00,
        0x00, 0x0F, 0xAC, 0x04,
        0x01, 0x00,
        0x00, 0x0F, 0xAC, 0x02,
        0x00, 0x00,
    };
    static const uint8_t rsn_ie_wpa3[] = {
        0x30, 0x14,
        0x01, 0x00,
        0x00, 0x0F, 0xAC, 0x04,
        0x01, 0x00,
        0x00, 0x0F, 0xAC, 0x04,
        0x01, 0x00,
        0x00, 0x0F, 0xAC, 0x08,
        0xC0, 0x00,
    };

    switch (auth) {
        case WIFI_AUTH_WPA_PSK:
            frame.insert(frame.end(), wpa_ie, wpa_ie + sizeof(wpa_ie));
            break;
        case WIFI_AUTH_WPA2_PSK:
            frame.insert(frame.end(), rsn_ie_wpa2, rsn_ie_wpa2 + sizeof(rsn_ie_wpa2));
            break;
        case WIFI_AUTH_WPA_WPA2_PSK:
            frame.insert(frame.end(), wpa_ie,      wpa_ie      + sizeof(wpa_ie));
            frame.insert(frame.end(), rsn_ie_wpa2, rsn_ie_wpa2 + sizeof(rsn_ie_wpa2));
            break;
        case WIFI_AUTH_WPA3_PSK:
        case WIFI_AUTH_WPA2_WPA3_PSK:
            frame.insert(frame.end(), rsn_ie_wpa3, rsn_ie_wpa3 + sizeof(rsn_ie_wpa3));
            break;
        default:
            break;
    }
}

void module::beacon::send_frame(const uint8_t* bssid, const char* ssid, uint8_t channel, wifi_auth_mode_t authmode) {
    uint8_t ssid_len = static_cast<uint8_t>(strnlen(ssid, 32));

    beacon_frame_t hdr;
    memcpy(hdr.sa, bssid, 6);
    memcpy(hdr.bssid, bssid, 6);

    if (authmode == WIFI_AUTH_OPEN) {
        hdr.capability[0] = 0x21;
        hdr.capability[1] = 0x04;
    } else {
        hdr.capability[0] = 0x31;
        hdr.capability[1] = 0x04;
    }

    std::vector<uint8_t> frame;
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&hdr);
    frame.insert(frame.end(), p, p + sizeof(hdr));

    // SSID IE (tag=0)
    frame.push_back(0x00);
    frame.push_back(ssid_len);
    frame.insert(frame.end(), reinterpret_cast<const uint8_t*>(ssid),
                              reinterpret_cast<const uint8_t*>(ssid) + ssid_len);

    // Supported Rates IE (tag=1)
    const uint8_t rates[] = {0x01, 0x08,
                              0x82, 0x84, 0x8b, 0x96,
                              0x0c, 0x12, 0x18, 0x24};
    frame.insert(frame.end(), rates, rates + sizeof(rates));

    // DS Parameter Set IE (tag=3)
    frame.push_back(0x03);
    frame.push_back(0x01);
    frame.push_back(channel);

    append_security_ies(frame, authmode);

    esp_wifi_80211_tx(WIFI_IF_STA, frame.data(), frame.size(), false);
}
