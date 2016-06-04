
#ifndef Semantic_h
  #define Semantic_h

  #include <Arduino.h>
  #include <ArduinoJson.h>
  #include <avr/pgmspace.h>
  #include <EEPROM.h>
  /******************************************************************************
  ***********************CONFIG**************************************************
  ******************************************************************************/
  //RESOURCES
  #define temperature_
  #define light_

  //OPERATIONS
  void addOperationFunction(void (*foo)(uint8_t, uint8_t[6], JsonObject&, uint8_t[6]), const char* id);
  #ifdef temperature_
    #define tempSense_
    #ifdef tempSense_
      void tempSense_f(uint8_t pin_count, uint8_t pins[6], JsonObject& root, uint8_t results[6]);
    #endif
  #endif
  #ifdef light_
    #define lightSwitch_
    #ifdef lightSwitch_
      void lightSwitch_f(uint8_t pin_count, uint8_t pins[6], JsonObject& root, uint8_t results[6]);
    #endif
  #endif

  /******************************************************************************/
  const char ERROR[] PROGMEM = "{"
    "\"@context\":\"_context_\","
    "\"@type\":\"Error\","
    "\"EntryPoint\":\"http://localhost:8080/\""
  "}";

  const char CAPABILITIES[] PROGMEM =
    "{"
      "\"@context\": \"_context_\","
      "\"@id\": \"/\","
      "\"@type\": \"hydra:EntryPoint\","
      "\"capabilities\": ["
        "{"
          "\"@id\": \"vocab:CapabilityEepromManagement\","
          "\"@type\": [\"hydra:Resource\", \"vocab:Capability\"],"
          "\"supportedOperation\":"
          "["
            "{"
              "\"@id\": \"eepromReset\","
              "\"@type\": \"hydra:Operation\","
              "\"method\": \"POST\","
              "\"label\": \"eepromReset\","
              "\"expects\": null,"
              "\"returns\": null"
            "},"
            "{"
              "\"@id\": \"eepromAdd\","
              "\"@type\": \"hydra:Operation\","
              "\"method\": \"POST\","
              "\"label\": \"eepromAdd\","
              "\"expects\": \"EepromEntry\","
              "\"returns\": null"
            "}"
            /*",{"
              "\"@id\": \"eepromRemove\","
              "\"@type\": \"hydra:Operation\","
              "\"method\": \"POST\","
              "\"label\": \"eepromRemove\","
              //"\"description\": \"Retrieves a temperature measured by the temperature sensor\","
              "\"expects\": \"EepromEntry\","
              "\"returns\": null"
            "}"*/
          "]"
        "},"
        "{"
        #ifdef temperature_
          "\"@id\": \"vocab:CapabilityTemperatureSense\","
          "\"@type\": [\"hydra:Resource\", \"vocab:Capability\"],"
          "\"supportedOperation\": ["
            "{"
            #ifdef tempSense_
              "\"@id\": \"tempSense\","
              "\"@type\": \"hydra:Operation\","
              "\"method\": \"GET\","
              "\"label\": \"temperatureSense\","
              "\"expects\": null,"
              "\"returns\": \"Temperature\""
            #endif
            "}"

          "]"
        #endif
        "},"
        "{"
        #ifdef light_
          "\"@id\": \"vocab:CapabilitySwitchLight\","
          "\"@type\": [\"hydra:Resource\", \"vocab:Capability\"],"
          "\"supportedOperation\": ["
            "{"
              "\"@id\": \"lightSwitch\","
              "\"@type\": \"hydra:Operation\","
              "\"method\": \"POST\","
              "\"label\": \"LightSwitch\","
              "\"expects\": \"Light\","
              "\"returns\": \"Light\""
            "}"
          "]"
        #endif
        "}"
      "]"
    "}\0"
  ;

  ////////////////////////////////////////////////////////////////
  // RESOURCES
  ///////////////////////////////////////////////////////////////
  const char TEMPERATURE[] PROGMEM=
    "{"
      #ifdef temperature_
      "\"@context\": \"APICONTEXT\","
      "\"@id\": \"temperatureSense\","
      "\"@type\": \"Temperature\","
      "\"value\": \"\","
      "\"type\": \"°C\""
      //c'est une uri ^
      #endif
    "}\0"
  ;


  const char LIGHT[] PROGMEM=
    "{"
      #ifdef light_
      "\"@context\": \"APICONTEXT\","
      "\"@id\": \"switchLight\","
      "\"@type\": \"Light\","
      "\"value\": \"\""
      #endif
    "}\0"
  ;

   const char EEPROMENTRY[] PROGMEM=
   "{"
     #ifdef light_
     "\"@context\": \"APICONTEXT\","
     "\"@id\": \"eepromEntry\","
     "\"@type\": \"EepromEntry\","
     "\"pin_count\": \"\","
     "\"pins\": \"\","
     "\"uri\":\"\""
     #endif
   "}\0"
 ;


  #define RESOURCE_COUNT 3

  PGM_P const json_resources[] PROGMEM =
  {
    LIGHT,
    TEMPERATURE,
    EEPROMENTRY
  };
#endif
