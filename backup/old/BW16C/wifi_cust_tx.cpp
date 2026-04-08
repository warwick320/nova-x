#include "wifi_cust_tx.h"

/*
 * Transmits a raw 802.11 frame with a given length.
 * The frame must be valid and have a sequence number of 0 as it will be set automatically.
 * The frame check sequence is added automatically and must not be included in the length.
 * @param frame A pointer to the raw frame
 * @param size The size of the frame
*/
void wifi_tx_raw_frame(void* frame, size_t length) {
  uint8_t *ptr = (uint8_t *)**(uint32_t **)(rltk_wlan_info + 0x10);
  uint8_t *frame_control = (uint8_t *)alloc_mgtxmitframe(ptr + 0xae0);

  if (frame_control != 0) {
    update_mgntframe_attrib(ptr, frame_control + 8);
    memset((void *)*(uint32_t *)(frame_control + 0x80), 0, 0x68);
    uint8_t *frame_data = (uint8_t *)*(uint32_t *)(frame_control + 0x80) + 0x28;
    memcpy(frame_data, frame, length);
    *(uint32_t *)(frame_control + 0x14) = length;
    *(uint32_t *)(frame_control + 0x18) = length;
    dump_mgntframe(ptr, frame_control);
  }
}

/*
 * Transmits a 802.11 deauth frame on the active channel
 * @param src_mac An array of bytes containing the mac address of the sender. The array has to be 6 bytes in size
 * @param dst_mac An array of bytes containing the destination mac address or FF:FF:FF:FF:FF:FF to broadcast the deauth
 * @param reason A reason code according to the 802.11 spec. Optional 
*/
void wifi_tx_deauth_frame(void* src_mac, void* dst_mac, uint16_t reason) {
  DeauthFrame frame;
  memcpy(&frame.source, src_mac, 6);
  memcpy(&frame.access_point, src_mac, 6);
  memcpy(&frame.destination, dst_mac, 6);
  frame.reason = reason;
  wifi_tx_raw_frame(&frame, sizeof(DeauthFrame));
}

/*
 * Transmits a very basic 802.11 beacon with the given ssid on the active channel
 * @param src_mac An array of bytes containing the mac address of the sender. The array has to be 6 bytes in size
 * @param dst_mac An array of bytes containing the destination mac address or FF:FF:FF:FF:FF:FF to broadcast the beacon
 * @param ssid '\0' terminated array of characters representing the SSID
*/
void wifi_tx_beacon_frame(void* src_mac, void* dst_mac, const char *ssid) {
  BeaconFrame frame;
  memcpy(&frame.source, src_mac, 6);
  memcpy(&frame.access_point, src_mac, 6);
  memcpy(&frame.destination, dst_mac, 6);
  for (int i = 0; ssid[i] != '\0'; i++) {
    frame.ssid[i] = ssid[i];
    frame.ssid_length++;
  }
  wifi_tx_raw_frame(&frame, 38 + frame.ssid_length);
}
#include <string.h>

void wifi_tx_beacon_frame_Privacy_RSN_IE(void* src_mac, void* dst_mac, const char *ssid) {
    uint8_t beacon_buffer[256] = {0};
    size_t offset = 0;

    // MAC Header
    beacon_buffer[0] = 0x80;  // Beacon Type
    beacon_buffer[1] = 0x00;
    memcpy(beacon_buffer + 4, dst_mac, 6);
    memcpy(beacon_buffer + 10, src_mac, 6);
    memcpy(beacon_buffer + 16, src_mac, 6);
    offset += 24;

    memset(beacon_buffer + offset, 0, 8);  // Timestamp
    beacon_buffer[offset + 8] = 0x64;  // Beacon Interval
    beacon_buffer[offset + 9] = 0x00;
    beacon_buffer[offset + 10] = 0x11;  // ESS + Privacy
    beacon_buffer[offset + 11] = 0x00;
    offset += 12;

    // SSID IE
    beacon_buffer[offset++] = 0x00;
    beacon_buffer[offset++] = strlen(ssid);
    memcpy(beacon_buffer + offset, ssid, strlen(ssid));
    offset += strlen(ssid);

    // RSN IE (WPA2-PSK, AES)
    uint8_t rsn_ie[] = {
        0x30, 20,
        0x01, 0x00,
        0x00, 0x0F, 0xAC, 0x04,
        0x01, 0x00,
        0x00, 0x0F, 0xAC, 0x04,
        0x01, 0x00,
        0x00, 0x0F, 0xAC, 0x02,
        0x00, 0x00
    };
    memcpy(beacon_buffer + offset, rsn_ie, sizeof(rsn_ie));
    offset += sizeof(rsn_ie);

    wifi_tx_raw_frame(beacon_buffer, offset);
}

void wifi_tx_auth_frame(void* src_mac, void* dst_mac, uint16_t seq) {
    AuthFrame frame;
    memcpy(&frame.source, src_mac, 6);
    memcpy(&frame.access_point, dst_mac, 6);
    memcpy(&frame.destination, dst_mac, 6);
    frame.sequence_number = seq;
    
    wifi_tx_raw_frame(&frame, sizeof(AuthFrame));
}

void wifi_tx_assoc_frame(void* src_mac, void* dst_mac, const char *ssid, uint16_t seq) {
    AssocFrame frame;
    memcpy(&frame.source, src_mac, 6);
    memcpy(&frame.access_point, dst_mac, 6);
    memcpy(&frame.destination, dst_mac, 6);
    frame.sequence_number = seq;
    
    frame.ssid_length = 0;
    for (int i = 0; ssid[i] != '\0' && i < 32; i++) {
        frame.ssid[i] = ssid[i];
        frame.ssid_length++;
    }
    
    size_t frame_size = 24 + 4 + 2 + frame.ssid_length + 2 + 1;
    wifi_tx_raw_frame(&frame, frame_size);
}