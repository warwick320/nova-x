#undef max
#undef min
#include "Display.h"


void bw_16_set_up(){
  Serial.begin(115200);
  setupButtons();
  u8g2.begin();
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);
  //u8g2.setFont(u8g2_font_boutique_bitmap_7x7_t_chinese1);
  //u8g2.setFont(u8g2_font_boutique_bitmap_7x7_t_gb2312b);
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 0, 128, 64, LOGO);
  u8g2.sendBuffer();
  delay(1000);
  main_names = {"攻击模式", "扫描网络", "选择目标","设置"};
  at_names = {"认证攻击", "全网断开攻击","信标帧","洪水攻击","关联洪水"};
  Settings = {"认证帧/次","信标帧/次","Max克隆数 ","Max干扰数"};
  becaon_names = {"垃圾广播","克隆"};
  Serial.println("display_set_up done");
  draw_menu(main_names, 0);
}
