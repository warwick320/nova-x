#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <vector>
#include <string>
#include <memory>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "WiFi.h"
#include "esp_wifi.h"

// Blue Tooth Adv
#define MAX_TX_POWER ESP_PWR_LVL_P20

// SSD1306 
#define OLED_RESET -1
#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 128

// Serial
#define SERIAL_SPEED 115200


struct BSSIDInfo;
const int I2C_SDA = 26;
const int I2C_SCL = 25;

const uint8_t BTN_UP = 24;
const uint8_t BTN_DOWN = 23;
const uint8_t BTN_OK = 28;
const uint8_t BTN_BACK = 10;

const int btnCount = 4;

const std::string prefix = "async"; // Change it to the prefix you want!

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C display;
extern std::map<uint8_t, std::vector<BSSIDInfo>> channelAPMap;
extern uint8_t totalAPCount;
class button;

extern button btnUp;
extern button btnDown;
extern button btnOk;
extern button btnBack;

extern button btns[];

