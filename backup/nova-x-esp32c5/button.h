#pragma once
#include <Arduino.h>
struct button {
  uint8_t pin;
  unsigned long lastDebounceTime;
  int state = HIGH;
  int lastState = HIGH;
  button(uint8_t p) : pin(p),lastDebounceTime(0),state(HIGH),lastState(HIGH) {}
};

namespace nx{
  class buttons{
    public:
    void setupButtons(button pins[],int numPins);
    bool btnPress(button &btn,unsigned long debounceDelay = 25);
  };
};