#include "button.h"

// setup button pins (input pullup)
void nx::buttons::setupButtons(button pins[],int numPins){
  for (int i = 0; i < numPins; i++) pinMode(pins[i].pin , INPUT_PULLUP);

}


// button press handler with debounce

bool nx::buttons::btnPress(button &btn,unsigned long debounceDelay){
  int reading = digitalRead(btn.pin);
  if (reading != btn.lastState) btn.lastDebounceTime = millis();
  if ((millis() - btn.lastDebounceTime) > debounceDelay) if (reading != btn.state){
    btn.state = reading;
    if(btn.state == LOW){
      #ifdef DEBUG_BUTTON
      Serial.print("Button Pressed: ");
      Serial.println(btn.pin);
      #endif
      btn.lastState = reading;
      return true;
    }
  }
  btn.lastState = reading;
  return false;
}