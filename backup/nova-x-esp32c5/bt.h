#pragma once
#include "globals.h"
#include "devices.h"
#include "NimBLEDevice.h"
#include "NimBLEAdvertising.h"

namespace nx{
  class bt{
    private:
    NimBLEAdvertising *advPtr = nullptr;
    NimBLEServer *btServerPtr = nullptr;
    uint8_t nullAddr[6] = {0xFE,0xED,0xC0,0xFF,0xEE,0x69};
    uint8_t dummyAddr[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
    void addrReset();
    void advTypeSet();
    const int LEN = 15;
    public:
    void init();
    void iosAdv();
    void samsungAdv();
    //void allAdv();
  };
}