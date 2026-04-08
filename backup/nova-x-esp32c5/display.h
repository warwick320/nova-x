#pragma once
#include "globals.h"
#include <functional>
#include <string>
#include <vector>
#include "button.h"
#include "wifiP.h"
#include "bitmap.h"
#include "bt.h"

// Beacon
#define MAX_SSID 60
#define MAX_SSID_LEN 13
#define CLONES_PER_AP 10

// packet monitor
#define PACKET_BAR_INTERVAL 100 // 100 ms -> 10FPS

struct menuItem {
  std::string name;
  std::function<void()> action;
  std::vector<menuItem> subMenu;
  menuItem(const std::string n, std::function<void()> act, std::vector<menuItem> sub = {}): name(n), action(act), subMenu(std::move(sub)) {}
};

struct BorderLine {
  int x1, y1, x2, y2;
  int step;
};

namespace nx {
  class menu {
  private:
    static constexpr uint8_t channels5G[] = {36, 40, 44, 48, 149, 153, 157, 161, 165};
        
    static constexpr const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";
    static constexpr const char* info[] = {"v1.4.1-beta", "ESP32-C5", "warwick320"};
    static constexpr int yOffsetTyping[] = {30, 40, 50}; 

    static constexpr const int SCROLL_DELAY = 1500;
    static constexpr const int SCROLL_DURATION = 1000;
    static constexpr const uint8_t MAX_STR_DISPLAY_LEN = 10;
    

    std::string lastTitle = "";
    bool fadeFlag = false;
    bool popupFlag = false;
    bool packetMonitorF = true;
    bool renderTypingF = true;

    std::vector<std::string> prevMenuName;
    int prevSelected = -1;
    int menuSize = 0;
    int visibleWidth = 0;
    int fontAscent = 0;
    int fontDescent = 0;
    int fontHeight = 0;

    unsigned long displayDuration = 2000;

    static constexpr uint8_t maxBrLevel = 15;
    static constexpr uint8_t framesPerStep = startAnimeallArray_LEN / 16 + 1;

    static constexpr int charWidth = 6;
    static constexpr int lineHeight = 15;
    static constexpr int baseX = 5;
    static constexpr int maxVisible = 4;
    static constexpr int cornerRadius = 5;
    static constexpr int minRectWidth = 40;
    static constexpr int textOffsetY = 3;
    static constexpr int padding = 10;
    static constexpr int scrollbarW = 3;

    static constexpr unsigned long animDuration = 300UL;
    static constexpr unsigned long lineAnimDuration = 250UL;
    static constexpr unsigned long lineDelay = 80UL;
    
    static constexpr int checkboxX = baseX + 2;
    static constexpr int checkboxSize = 8;
    static constexpr int ssidStartX = checkboxX + 12;
    static constexpr int rectStartX = ssidStartX - 2;
    static constexpr int channelRightMargin = baseX + 2;
    static constexpr int channelLeftPadding = 4;

    int startX = 33;
    int endX = 79;
    int startY = 41;
    int progress = 0;
    int arcHeight = 20;

    unsigned long lastSelectionTime = 0;
    bool isScrolling = false;
    int scrollOffset = 0;
    int lastScrollIndex = -1;
    
    
    nx::buttons btn;
    nx::wifi tx;
    nx::bt adv;

    void debug_print(const std::string &ctx);

    void setBrightness(uint8_t brightness);

    void fadeIn();
    void fadeOut();

    int calcStartIndex(int sel);
    int calcRectWidth(int textWidth);
    int calcRectX(int rectWidth);
    int calcTextX(int rectX, int rectWidth, int textWidth);
    int calcTextY(int y);
    int calcCheckboxRectWidth(const std::string &channel);
    
    void renderRoundRect(int x, int y, int w, int h);
    void renderCheckbox(int x, int y, bool checked);
    void renderLineWithCheckbox(const std::string &ssid, const std::string &channel, int y, bool checked, bool selected,int itemIndex = -1);
    void renderSingleLine(const std::string &text, int y, bool drawRect);
    void renderText(const std::vector<std::string> &items, int start, int y, int count);
    void renderPopup(const std::string& ctx);
    void renderTyping(const char* const* texts, const int* yPositions, int textCount, unsigned long displayDuration, std::function<void()> drawBackground);
    void renderProgressEffect(int progress);
    void renderScanEffect(int progress);

    void drawBorder();
    void drawInitialAnimation(const std::vector<std::string> &items, int idx);
    void drawSelectionAnimation(const std::vector<std::string> &items, int newIdx);
    void drawStaticFrame(const std::vector<std::string> &items, int idx);
    void drawCheckboxInitialAnimation(const std::vector<std::string> &ssids, const std::vector<std::string> &channels, const std::vector<bool> &checked, int idx);
    void drawCheckboxSelectionAnimation(const std::vector<std::string> &ssids, const std::vector<std::string> &channels, const std::vector<bool> &checked, int newIdx);
    void drawCheckboxFrame(const std::vector<std::string> &ssids, const std::vector<std::string> &channels, const std::vector<bool> &checked, int idx);

    void runBle(std::function<void()> func, const std::string& title);

    std::vector<std::string> getChannelList();
    void getSSIDAndChannelLists(std::vector<std::string> &ssids, std::vector<std::string> &channels);

    std::string formatBSSID(const uint8_t* bssid);
    std::string truncateSSID(const std::string& ssid, size_t maxLen = 10);

    uint8_t getRandomChannel();
    void generateRandomBSSID(uint8_t* bssid);
    std::vector<std::string> getRandomSSID();
    std::vector<std::array<uint8_t,6>> generateRandomBSSIDList();
    
    //exec
    void executeAttackAll(const std::string& title,std::function<void()> attackFunc);
    void executeChannelAttack(const char* attackType, std::function<void(uint8_t)> attackFunc);
    void executeSelectedAttack(const char* attackType, std::function<void(const BSSIDInfo&, uint8_t)> attackFunc);
    void executeBeaconAttack(const std::vector<const BSSIDInfo*>& apList, const std::string& title);
    void executeSelectMenu(std::vector<std::string>& items,std::vector<std::string>& channels,std::vector<bool>& selectedFlags,const std::string& emptyMessage);

    void getSTALists(std::vector<std::string> &macs,std::vector<std::string> &channels);
  
    //Scroll 
    std::string getDisplaySSID(const std::string& ssid,int idx,bool isSelected);
    void updateScrollState(int selectedIdx);

  public:
    int index = 0;
    std::vector<bool> selectedAPs;
    std::vector<bool> selectedSTAs;
    // Start Animation
    void startAnimation();
    // Core
    void init();
    void drawSubMenu(const std::string& title, bool progressFlag = true, bool scanFlag = false);
    void drawMenu(const std::vector<std::string> &items, int index);
    void menuHandler(std::vector<menuItem> &menu, int index);

    // Scan
    void scanWiFi();
    void scanSTA();

    // ==== Attack ====

    // BlueTooth ADV
    void drawIosAdv();
    void drawSamsungAdv();
    // Deauth
    void deauthAttack();
    void deauthByChannel();
    void deauthSelected();
    // Beacon
    void beaconAllSSID();
    void beaconSSIDDupe();
    void beaconRandom();
    void beaconDupeByChannel();
    void beaconCustomPrefix(const std::string& prefix);
    // Auth
    void authAttack();
    void authByChannel();
    void authSelected();
    // Assoc
    void assocAttack();
    void assocByChannel();
    void assocSelected();
    // Deauth By STA
    void attackSelectedBySTAs();
    // AP Spoofing
    void APSpoofing();
    //void attackSelectedBySTAsByChannel();

    // ================

    // PacketMonitor
    void drawPacketMonitor();

    // Settings
    void drawSelectMenu();
    void drawMenuWithCheckbox(const std::vector<std::string> &ssids, const std::vector<std::string> &channels, const std::vector<bool> &checked, int index);
    void drawSelectMenuSTA();
    // About
    void drawAbout();
  };

}


