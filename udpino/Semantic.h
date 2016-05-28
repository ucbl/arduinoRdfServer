
#ifndef Semantic_h
#define Semantic_h

#include <Arduino.h>
#include <avr/pgmspace.h>
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
      #ifdef temperature_
        "\"@id\": \"vocab:CapabilityTemperatureSense\","
        "\"@type\": [\"hydra:Resource\", \"vocab:Capability\"],"
        "\"subClassOf\": null,"
        "\"label\": \"CapabilityTemperatureSense\","
        //"\"description\": \"Capability that queries a temperature sensor\","
        "\"supportedOperation\": ["

          "{"
          #ifdef tempSense_
            "\"@id\": \"tempSense\","
            "\"@type\": \"hydra:Operation\","
            "\"method\": \"GET\","
            "\"label\": \"temperatureSense\","
            //"\"description\": \"Retrieves a temperature measured by the temperature sensor\","
            "\"expects\": null,"
            "\"returns\": \"Temperature\""
            //"\"statusCodes\": []"
          #endif
          "}"

        "]"
      #endif
      "},"
      "{"
      #ifdef light_
        "\"@id\": \"vocab:CapabilitySwitchLight\","
        "\"@type\": [\"hydra:Resource\", \"vocab:Capability\"],"
        "\"subClassOf\": null,"
        "\"label\": \"CapabilityLightSwitch\","
        //"\"description\": \"Capability that queries a temperature sensor\","
        "\"supportedOperation\": ["
          "{"
            "\"@id\": \"lightSwitch\","
            "\"@type\": \"hydra:Operation\","
            "\"method\": \"POST\","
            "\"label\": \"LightSwitch\","
            //"\"description\": \"Retrieves a temperature measured by the temperature sensor\","
            "\"expects\": \"Light\","
            "\"returns\": null,"
            "\"statusCodes\": []"
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
    "\"value\": \"\","
    "\"type\": \"°C\""
    #endif
  "}\0"
;

#define RESOURCE_COUNT 2

PGM_P const json_resources[] PROGMEM =
{
  LIGHT,
  TEMPERATURE
};


#endif
