
#ifndef Semantic_h
  #define Semantic_h

  #include <Arduino.h>
  #include <ArduinoJson.h>
  #include <avr/pgmspace.h>
  #include <EEPROM.h>
  /******************************************************************************
  CONFIG
  ******************************************************************************/
  //RESOURCES
  #define temperature_
  #define light_

  //OPERATIONS
  void addOperationFunction(void (*foo)(uint8_t, uint8_t[6], JsonObject&, uint8_t[6]), const char* id);
  #ifdef temperature_
      void tempSense_f(uint8_t pin_count, uint8_t pins[6], JsonObject& root, uint8_t results[6]);
  #endif
  #ifdef light_
      void lightSwitch_f(uint8_t pin_count, uint8_t pins[6], JsonObject& root, uint8_t results[6]);
  #endif

  /*****************************************************************************
  ENTRYPOINT
  ******************************************************************************/

  const char CAPABILITIES[] PROGMEM =
    "{"
      "\"@context\": {"
       "\"hydra\": \"http://www.w3.org/ns/hydra/core#\""
      "},"
      "\"@id\": \"/\","
      "\"@type\": \"hydra:EntryPoint\","
      "\"capabilities\": ["
        "{"
          "\"@id\": \"vocab:CapabilityEepromManagement\","
          "\"@type\": [\"hydra:Resource\", \"vocab:Capability\"],"
          "\"supportedOperation\":"
          "["
            "{"
              "\"@id\": \"reset\","
              "\"@type\": \"hydra:Operation\","
              "\"method\": \"PUT\","
              "\"label\": \"reset\","
              "\"expects\": null,"
              "\"returns\": null"
            "},"
            "{"
              "\"@id\": \"enable\","
              "\"@type\": \"hydra:Operation\","
              "\"method\": \"PUT\","
              "\"label\": \"enable\","
              "\"expects\": \"EepromEntry\","
              "\"returns\": null"
            "}"
          "]"
        "},"
        "{"
        #ifdef temperature_
          "\"@id\": \"vocab:CapabilityTemperatureSense\","
          "\"@type\": [\"hydra:Resource\", \"vocab:Capability\"],"
          "\"supportedOperation\": ["
            "{"
              "\"@id\": \"tempSense\","
              "\"@type\": \"hydra:Operation\","
              "\"method\": \"GET\","
              "\"label\": \"temperatureSense\","
              "\"expects\": null,"
              "\"returns\": \"Temperature\""
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

  /*****************************************************************************
  ERROR MESSAGE
  ******************************************************************************/
  const char ERROR[] PROGMEM =
    "{"
      "\"@context\":\"_context_\","
      "\"@type\":\"Error\","
      "\"EntryPoint\":\"http://localhost:8080/\""
    "}"
  ;

  /*****************************************************************************
  RESOURCES
  Variables are written instead of double \"\"
  ******************************************************************************/
  const char TEMPERATURE[] PROGMEM=
    "{"
      #ifdef temperature_
      "\"@context\": \"APICONTEXT\","
      "\"@id\": \"temperatureSense\","
      "\"@type\": \"Temperature\","
      "\"value\": \"\","
      "\"type\": \"°C\""
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
