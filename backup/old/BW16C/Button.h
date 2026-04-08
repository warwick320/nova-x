struct Button {
  uint8_t pin;
  unsigned long lastDebounceTime = 0;
  int state = HIGH;
  int lastState = HIGH;
  Button(uint8_t p)
    : pin(p), lastDebounceTime(0), state(HIGH), lastState(HIGH) {}
};
#define BTN_DOWN PA27
#define BTN_UP PA12
#define BTN_OK PA13
#define BTN_BACK PB3

Button btnDown(BTN_DOWN);
Button btnUp(BTN_UP);
Button btnOk(BTN_OK);
Button btnBack(BTN_BACK);

void setupButtons() {
  pinMode(btnDown.pin, INPUT_PULLUP);
  pinMode(btnUp.pin, INPUT_PULLUP);
  pinMode(btnOk.pin, INPUT_PULLUP);
  pinMode(btnBack.pin, INPUT_PULLUP);
}
bool ButtonPress(Button& btn, unsigned long debounceDelay = 25) {
  int reading = digitalRead(btn.pin);
  if (reading != btn.lastState) {
    btn.lastDebounceTime = millis();
  }

  if ((millis() - btn.lastDebounceTime) > debounceDelay) {
    if (reading != btn.state) {
      btn.state = reading;
      if (btn.state == LOW) {
        btn.lastState = reading;
        return true;
      }
    }
  }

  btn.lastState = reading;
  return false;
}
