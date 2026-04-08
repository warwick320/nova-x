#include "globals.h"
#include "button.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(
  U8G2_R0,
  OLED_RESET
);

button btnUp(BTN_UP);
button btnDown(BTN_DOWN);
button btnOk(BTN_OK);
button btnBack(BTN_BACK);

button btns[] = {btnUp,btnDown,btnOk,btnBack};
