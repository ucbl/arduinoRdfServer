#ifndef Semantic_h
#define Semantic_h
/******************************************************************************
***********************CONFIG**************************************************
******************************************************************************/
//RESOURCES
#define temperature_
#define light_

//OPERATIONS
#ifdef temperature_
  #define tempSense_
#endif
#ifdef light_
  #define lightSwitch_
#endif



#include <Arduino.h>
#include <avr/pgmspace.h>

const char ERROR[] PROGMEM = "{"
  "\"@context\":\"_context_\","
  "\"EntryPoint\":\"http://localhost:8080/\""
"}";

const char CAPABILITIES[] PROGMEM =
  "{"
    "\"@context\": \"_context_\","
    "\"@id\": \"/\","
    "\"@type\": \"hydra:EntryPoint\","
    "\"capabilities\": ["
      #ifdef temperature_
      "{"
        "\"@id\": \"vocab:CapabilityTemperatureSense\","
        "\"@type\": [\"hydra:Resource\", \"vocab:Capability\"],"
        "\"subClassOf\": null,"
        "\"label\": \"CapabilityTemperatureSense\","
        "\"description\": \"Capability that queries a temperature sensor\","
        "\"supportedOperation\": ["
          #ifdef tempSense_
          "{"
            "\"@id\": \"_:temperatureSense\","
            "\"@type\": \"hydra:Operation\","
            "\"method\": \"GET\","
            "\"label\": \"temperatureSense\","
            //"\"description\": \"Retrieves a temperature measured by the temperature sensor\","
            "\"expects\": null,"
            "\"returns\": \"vocab:Temperature\""
            //"\"statusCodes\": []"
          "}"
          #endif
        "]"
      "}"
      #endif
      #ifdef light_
      ",{"
        "\"@id\": \"vocab:CapabilitySwitchLight\","
        "\"@type\": [\"hydra:Resource\", \"vocab:Capability\"],"
        "\"subClassOf\": null,"
        "\"label\": \"CapabilityLightSwitch\","
        "\"description\": \"Capability that queries a temperature sensor\","
        "\"supportedOperation\": ["
          "{"
            "\"@id\": \"_:lightSwitch\","
            "\"@type\": \"hydra:Operation\","
            "\"method\": \"POST\","
            "\"label\": \"LightSwitch\","
            //"\"description\": \"Retrieves a temperature measured by the temperature sensor\","
            "\"expects\": \"vocab:Light\","
            "\"returns\": \"vocab:Light\","
            "\"statusCodes\": []"
          "}"
        "]"
      "}"
      #endif
    "]"
  "}\0"
;

////////////////////////////////////////////////////////////////
// RESOURCES
///////////////////////////////////////////////////////////////
#ifdef temperature_
const char TEMPERATURE[] PROGMEM=
  "{"
    "\"@context\": \"APICONTEXT\","
    "\"@id\": \"/temperatureSense\","
    "\"@type\": \"Temperature\","
    "\"value\": #,"
    "\"type\": \"°C\""
  "}\0"
;
#endif


const char LIGHT[] PROGMEM=

  "{"
    #ifdef light
    "\"@context\": \"APICONTEXT\","
    "\"@id\": \"/switchLight\","
    "\"@type\": \"Light\","
    "\"value\": #,"
    "\"type\": \"°C\""
    #endif
  "}\0"
;

#endif
