#ifndef JsonParser_h
#define JsonParser_h

#include <Arduino.h>
#include <ArduinoJson.h>


class JsonParser
{
  public:
    JsonParser();
    void parseChunk(String chunk);
};


#endif
