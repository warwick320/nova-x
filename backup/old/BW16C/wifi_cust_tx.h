#ifndef WIFI_CUST_TX
#define WIFI_CUST_TX

#include <Arduino.h>

typedef struct {
  uint16_t frame_control = 0xC0;
  uint16_t duration = 0xFFFF;
  uint8_t destination[6];
  uint8_t source[6];
  uint8_t access_point[6];
  const uint16_t sequence_number = 0;
  uint16_t reason = 0x06;
} DeauthFrame;

typedef struct {
  uint16_t frame_control = 0x80;
  uint16_t duration = 0;
  uint8_t destination[6];
  uint8_t source[6];
  uint8_t access_point[6];
  const uint16_t sequence_number = 0;
  const uint64_t timestamp = 0;
  uint16_t beacon_interval = 0x64;
  uint16_t ap_capabilities = 0x21;
  const uint8_t ssid_tag = 0;
  uint8_t ssid_length = 0;
  uint8_t ssid[255];
} BeaconFrame;

typedef struct {
    uint16_t frame_control = 0xB0;
    uint16_t duration = 0x013A;
    uint8_t destination[6];
    uint8_t source[6];
    uint8_t access_point[6];
    uint16_t sequence_number = 0;
    // Fixed Fields
    uint16_t auth_algorithm = 0x0000;
    uint16_t auth_seq = 0x0001;
    uint16_t status_code = 0x0000;
} AuthFrame;

typedef struct {
    uint16_t frame_control = 0x00;
    uint16_t duration = 0x0000;
    uint8_t destination[6];
    uint8_t source[6];
    uint8_t access_point[6];
    uint16_t sequence_number = 0;
    uint16_t capability_info = 0x0001;
    uint16_t listen_interval = 0x000A;
    const uint8_t ssid_tag = 0x00;
    uint8_t ssid_length = 0;
    uint8_t ssid[32];
    const uint8_t rates_tag = 0x01;
    const uint8_t rates_length = 1;
    uint8_t rates[1] = {0x8C}; 
} AssocFrame;

/*
 * Import the needed c functions from the closed-source libraries
 * The function definitions might not be 100% accurate with the arguments as the types get lost during compilation and cannot be retrieved back during decompilation
 * However, these argument types seem to work perfect
*/
extern uint8_t* rltk_wlan_info;
extern "C" void* alloc_mgtxmitframe(void* ptr);
extern "C" void update_mgntframe_attrib(void* ptr, void* frame_control);
extern "C" int dump_mgntframe(void* ptr, void* frame_control);

void wifi_tx_raw_frame(void* frame, size_t length);
void wifi_tx_deauth_frame(void* src_mac, void* dst_mac, uint16_t reason = 0x06);
void wifi_tx_beacon_frame(void* src_mac, void* dst_mac, const char *ssid);
void wifi_tx_beacon_frame_Privacy_RSN_IE(void* src_mac, void* dst_mac, const char *ssid);
void wifi_tx_assoc_frame(void* src_mac, void* dst_mac, const char *ssid, uint16_t seq);
void wifi_tx_auth_frame(void* src_mac, void* dst_mac, uint16_t seq);

#endif