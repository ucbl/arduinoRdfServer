#include "Semantic.h"

void lightSwitch_f(uint8_t pin_count, uint8_t pins[6], JsonObject& root, uint8_t results[6]){
  Serial.println(F("lightSwitch"));
  if(pin_count == 1){
    if(strcmp((const char*)root["@type"],"Light")==0){
      if(strcmp((const char*)root["value"],"off")==0){
        digitalWrite(pins[0], LOW);
        Serial.println(F("Turning light off"));
        results[0]=1;
      }
      if(strcmp((const char*)root["value"],"on")==0){
        digitalWrite(pins[0], HIGH);
        Serial.println(F("Turning light on"));
        results[0]=2;
      }
    }
  //  digitalWrite(pins[0],value);
  } else
    results[0]=NULL;
}

void tempSense_f(uint8_t pin_count, uint8_t pins[6], JsonObject& root, uint8_t results[6]){
  if(pin_count == 1)
    Serial.println(F("GOT TO THE FUNCTION TEMPSENSE"));
    results[0] = analogRead(pins[0]);
}
