#include "globals.h"
#include "display.h"

nx::menu m;

std:: vector<menuItem> mainMenu = {
  menuItem("Exploit",nullptr,{
    menuItem("Deauth", nullptr, {
      menuItem("APs",nullptr,{
        menuItem("All APs",[](){m.deauthAttack();},{}),
        menuItem("Channel",[](){m.deauthByChannel();},{}),
        menuItem("Selected",[](){m.deauthSelected();},{})
      }),
      menuItem("STAs",nullptr,{
        menuItem("Selected",[](){m.attackSelectedBySTAs();},{})
      })
    }),
    menuItem("Auth", nullptr, {
      menuItem("All APs",[](){m.authAttack();},{}),
      menuItem("Channel",[](){m.authByChannel();},{}),
      menuItem("Selected",[](){m.authSelected();},{})
    }),
    menuItem("Assoc", nullptr, {
      menuItem("All APs",[](){m.assocAttack();},{}),
      menuItem("Channel",[](){m.assocByChannel();},{}),
      menuItem("Selected",[](){m.assocSelected();},{})
    }),
    menuItem("Beacon", nullptr, {
      menuItem("All SSIDs Dupe",[](){m.beaconAllSSID();},{}),    
      menuItem("Selected Dupe",[](){m.beaconSSIDDupe();},{}),  
      menuItem("Random",[](){m.beaconRandom();},{}),
      menuItem("Channel",[](){m.beaconDupeByChannel();},{}),
      menuItem("Prefix",[](){m.beaconCustomPrefix(prefix);},{})
    }),
    menuItem("AP Spoofing", nullptr, {
      menuItem("Selected",[](){m.APSpoofing();},{})
    }),
    menuItem("B.T Adv",nullptr,{
      menuItem("Samsung",[](){m.drawSamsungAdv();},{}),
      menuItem("IOS",[](){m.drawIosAdv();},{})
    })
  }),
  menuItem("Scan",nullptr,{
    menuItem("APs",[](){m.scanWiFi();},{}),
    menuItem("STAs",[](){m.scanSTA();},{})
  }),
  menuItem("Packet Monitor",[](){m.drawPacketMonitor();},{}),
  menuItem("Settings",nullptr,{
    menuItem("Select APs",[](){m.drawSelectMenu();},{}),
    menuItem("Select STAs",[](){m.drawSelectMenuSTA();},{})
  }),
  menuItem("About",[](){m.drawAbout();},{}),
};

void setup(){
  Serial.begin(SERIAL_SPEED);
  Serial.println("[SERIAL] Started");
  Serial.println(F("=============================================================="));
  m.init();
  
}

void loop(){
  m.menuHandler(mainMenu,m.index);
}