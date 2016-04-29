#ifndef Semantic_h
#define Semantic_h

#include <Arduino.h>
#include <avr/pgmspace.h>

typedef struct{
  String path;
  const char* const_payload;
} uripayload_t;

const char ERROR[] PROGMEM = "Unknown URI";

const char CAPABILITIES[] PROGMEM =
  "{"
    "\"@context\": \"#context#\","
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
          "},"
          "{"
            "\"@id\": \"_:changePin\","
            "\"@type\": \"hydra:Operation\","
            "\"method\": \"POST\","
            "\"label\": \"temperatureSense\","
            "\"description\": \"Changes the Pin to look for the temperature\","
            "\"expects\": \"int\","
            "\"returns\": null,"
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
    "\"value\": #,"
    "\"type\": \"°C\""
  "}\0"
;


#endif
