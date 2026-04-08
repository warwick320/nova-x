#pragma once
#include <WiFi.h>
#include <cstdint>
#include <string>
#include <array>
#include <map>
#define STORE_LEN 64
#define PER_PKT 10

struct macAddr {
  uint8_t addr[6];
};

struct BSSIDInfo {
  std::array<uint8_t, 6> bssid;
  std::string ssid;
  bool encrypted; 
  
  BSSIDInfo(const uint8_t* mac, const char* name, bool enc = false) : ssid(name), encrypted(enc) {
    memcpy(bssid. data(), mac, 6);
  }
  
  bool operator==(const BSSIDInfo& other) const {
    return bssid == other.bssid;
  }
};

struct STAInfo{
  std::array<uint8_t,6> staMac;
  std::array<uint8_t,6> apMac;
  unsigned long lastSeen;
  uint32_t packetCount;

  STAInfo(const uint8_t* sta,const uint8_t* ap) : lastSeen(millis()) , packetCount(1){
    memcpy(staMac.data(),sta,6);
    memcpy(apMac.data(),ap,6);
  }
  bool operator==(const STAInfo& other) const {
    return staMac == other.staMac;
  }
  void update(){
    lastSeen = millis();
    packetCount++;
  }
};

extern std::map<uint8_t,std::vector<STAInfo>> channelSTAMap;
extern uint16_t totalSTACount; 

void addOrUpdateSTA(const uint8_t* staMac,const uint8_t* apMac , uint8_t channel);
int findSTA(const uint8_t* mac);

// packet
extern bool packetMonitorActive;

extern uint32_t packets;
extern uint32_t tmpDeauths;
extern uint32_t deauths;
extern uint32_t beaconPackets;
extern uint32_t probePackets;
extern uint32_t dataPackets;

extern unsigned long snifferStartTime;
extern unsigned long snifferPacketTime;
extern unsigned long snifferChannelTime;

extern bool channelHopping;

#define SCAN_PACKET_LIST_SIZE 60
#define PACKET_LIST_UPDATE_INTERVAL 1000 // 1 sec
#define PACKET_MONITOR_CHANNEL_HOPPING_INTERVAL 1000 // 1 sec

extern uint16_t packetList[SCAN_PACKET_LIST_SIZE];

extern uint16_t packetListIdx;

extern uint8_t currentChannelIdx;

extern bool useOnlyNonDFS; 

bool macBroadcast(const uint8_t* mac);
bool macMulticast(const uint8_t mac);
bool macValid(const uint8_t* mac);
int findAP(const uint8_t* mac);
bool isDFSChannel(uint8_t ch);
// Lib
namespace nx {
  class wifi {
  private:
    uint8_t currentChannel = 0;
    
    // Processes scan results by filtering duplicates and updating channel AP map
    void processScanResults(int scanCount, uint8_t channel, int& totalScan, bool verbose = false);
    
  public:
    static const uint8_t channelList[37];
    static const size_t channelCount;
    
    // Scan variables
    size_t channelIndex = 0;
    unsigned long lastScan = 0;
    unsigned long scanInterval = 300;
    bool scanComplete = false;

    size_t staChannelIndex = 0;
    unsigned long lastSTAScan = 0;
    unsigned long staScanInterval = 1500;
    bool staScanComplete = false;
    bool staScanActive = false;

    void init();
    void debug_print(const char* context);
    
    // MAC management
    void storeMac(const uint8_t* mac);
    bool checkedMac(const uint8_t* mac);
    void clearMacStored();

    // Channel management
    void setBandChannel(uint8_t channel);
    uint8_t getCurrentChannel() const{ return currentChannel;}
    void setChannelHopping(bool enable);
    bool getChannelHopping();
    void nextChannel();
    void prevChannel();
    void nextChannelToAP();
    void setUseDFS(bool enable);
    bool getUseDFS();

    // Scan functions
    int scanNetwork(bool all = true, uint8_t channel = 0);
    void performProgressiveScan();
    
    // Deauth
    void txDeauthFrameAll();
    void txDeauthFrameChannel(uint8_t channel);
    void txDeauthFrameBSSID(const uint8_t* bssid, uint8_t channel);
    
    // Beacon
    void txBeaconFrame(const char* ssid, uint8_t channel, const uint8_t* bssid, bool encrypted = false);
    
    
    // Authentication 
    void txAuthFrame(const uint8_t* bssid, uint8_t channel);
    void txAuthFlood();
    
    // Association 
    void txAssocFrame(const uint8_t* bssid, const char* ssid, uint8_t channel);
    void txAssocFlood();

    // Probe Response
    void txProbeResponse(const uint8_t* bssid,const char* ssid,uint8_t channel,const uint8_t* destMac);

    // Packet monitor
    void startPacketMonitor();
    void stopPacketMonitor();
    void updatePacketMonitor();

    bool isMonitoring();

    // Packet
    uint32_t getPacketRate();
    uint32_t getDeauthCount();
    uint32_t getBeaconCount();
    uint32_t getProbeCount();
    uint32_t getDataCount();
    uint16_t getPacketAtIdx(int idx);
    uint16_t getMaxPacket();
    double getScaleFactor(uint8_t height);

    //STA
    void clearSTAMap();
    uint8_t getSTACount();
    std::vector<STAInfo> getSTAsByChannel(uint8_t channel);
    std::vector<STAInfo> getSTAsByAP(const uint8_t* apMac);

    //STA scan
    void startSTAScan();
    void stopSTAScan();
    void performProgressiveSTAScan();
    int scanSTAOnChannel(uint8_t channel);
    // STA Targeted Deauth
    void txDeauthFrameSTA(const uint8_t* staMac,const uint8_t* apMac,uint8_t channel);
    // should i implement?
    void txDeauthFrameAllSTAs();
    void txDeauthFrameSTAsByChannel(uint8_t channel);

    //void printAllSTAs();
  };

}
