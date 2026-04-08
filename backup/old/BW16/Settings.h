#undef max
#undef min
#include "Display.h"
//#include ""
void logo_animate(){
  for(int i=0; i< Logo_gif_allArray_LEN; i++){
    display.drawBitmap(0,0,Logo_gif_allArray[i],128,64,SSD1306_WHITE);
    display.display();
    delay(45); // 장수 마다 다름 장수 나누기 60 나온 값에 10곱
  }
}
void bw_16_set_up(){
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 F"));
    while (true)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
  logo_animate();
  // display.drawBitmap(0,0,LOGO,128,64,SSD1306_WHITE);
  // display.display();
  delay(1500);
  setupButtons();
  //Wire.begin(); 버그 있음 3.1.8버전
  main_names = {"Attack", "Scan", "Select","Settings"};
  at_names = {"Deauth", "All Deauth","Beacon","Authentication","Association"};
  Serial.println("done");

  // if (scanNetworks("Booting") != 0) {
  //   while(true) delay(1000);
  // }
  //SendToEsp("BW16");
  draw_menu(main_names, 0);
}
