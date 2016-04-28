#ifndef Semantic_h
#define Semantic_h

#include <Arduino.h>
#include <avr/pgmspace.h>

typedef struct{
  String path;
  const char* const_payload;
  byte variables;
  int eeprom_port_index;
  int eeprom_uri_index;
} uripayload_t;

const char CAPABILITIES[] PROGMEM =
  "{"
    "\"@context\": \"THECONTEXT\","
    "\"@id\": \"/\","
    "\"@type\": \"hydra:EntryPoint\","
    "\"capabilities\": ["
      "{"
        "\"@id\": \"vocab:CapabilityTemperatureSense\","
        "\"@type\": [\"hydra:Resource\", \"vocab:Capability\"],"
        "\"subClassOf\": null,"
        "\"label\": \"CapabilityTemperatureSense\","
        "\"description\": \"Capability that queries a temperature sensor\","
        "\"supportedOperation\": ["
          "{"
            "\"@id\": \"_:temperatureSense\","
            "\"@type\": \"hydra:Operation\","
            "\"method\": \"GET\","
            "\"label\": \"temperatureSense\","
            "\"description\": \"Retrieves a temperature measured by the temperature sensor\","
            "\"expects\": null,"
            "\"returns\": \"vocab:Temperature\","
            "\"statusCodes\": []"
          "}"
        "]"
      "}"
    "]"
  "}\0"
;

const char TEMPERATURE[] PROGMEM=
  "{"
    "\"@context\": \"APICONTEXT\","
    "\"@id\": \"/temperature\","
    "\"@type\": \"Temperature\","
    "\"value\": null,"
    "\"type\": \"#\""
  "}\0"
;


#endif
