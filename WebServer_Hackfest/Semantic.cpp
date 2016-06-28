/*****************************************************************************
Functions to be linked to semantics in Webserver_hackfest.ino
******************************************************************************/
#include "Semantic.h"

void lightSwitch_f(uint8_t pin_count, uint8_t pins[6], JsonObject& root, uint8_t results[6]){
  Serial.print(F("lightSwitch pin "));
  Serial.println((int)pins[0]);
  if(pin_count == 1 && strcmp((const char*)root["@type"],"Light")==0){
    pinMode((int)pins[0], INPUT);
    if(strcmp_P((const char*)root["value"],PSTR("off"))==0){
      digitalWrite((int)pins[0], LOW);
      Serial.println(F("Turning light off"));
      results[0]=10;
      results[5]=0;//no error
    } else if(strcmp_P((const char*)root["value"],PSTR("on"))==0){
      digitalWrite((int)pins[0], HIGH);
      Serial.println(F("Turning light on"));
      results[0]=11;
      results[5]=0;//no error
    } else
      results[5]=1;// error
  } else{
    Serial.println(F("lightSwitch_f error"));
    results[5]=1;// error
  }
}

void tempSense_f(uint8_t pin_count, uint8_t pins[6], JsonObject& root, uint8_t results[6]){
  if(pin_count == 1)
    results[0] = (analogRead(pins[0])*0.4882814)-50;
}
