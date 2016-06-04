#include <Arduino.h>
#include "JsonParser.h"

JsonParser::JsonParser(){
}

void JsonParser::parseChunk(String chunk){
  if(chunk.indexOf(':')>=0){
    if(chunk.lastIndexOf('[')>chunk.lastIndexOf(':')) chunk+=']';
    chunk+='}';
    //once a chunk is complete, treat it with ArduinoJson
    StaticJsonBuffer<300> jsonBuff;
    JsonObject& root = jsonBuff.parse(chunk);
    if(!root.success()){
      Serial.print(F("error parsing: "));
      Serial.println(chunk);
    }
    // Testing if "haha" exist as a field in the JSON chunk
    const char* haha = root["haha"];
    if(haha) Serial.println(F("yes haha"));
    else Serial.println(F("no haha"));
  }
  chunk="{";
}
