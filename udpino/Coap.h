#ifndef Coap_h
#define Coap_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <EthernetUdp2.h>
#include "ResourceManager.h"
// *** type ***
#define ACK B00100000
#define CON B00000000
#define NON B00010000
// *** code ***
#define EMPTY B00000000
#define VALID B01000011
#define CREATED B01000001 //only for POST-PUT
#define DELETED B01000010 //only for POST-PUT
#define CHANGED B01000100
#define CONTENT B01000101 //only for GET
#define OPT_DELTA_EXTENDED B11010000
#define PAYLOAD_START B11111111
#define M B00001000
#define PAYLOAD_MAX_SIZE 64
#define OPT_DELTA_EXTENDED B11010000

class Coap
{
  public:
    ResourceManager* rsm;
    Coap(char* payloadBuffer, ResourceManager* rsm_p);
    int parseHeader();
    int packetCursor;
    int payloadCursor;
    int tokenLength;
    int uri[2];
    int optDelta;
    uint8_t payload_depth;
    uint8_t max_payload_depth;
    // Request
    uint8_t method;
    int payloadStartIndex_r;
    String payload_chunk;
    // Blocks
    int blockIndex;
    uint8_t blockType;
    char payloadData[10];
    void sendBuffer(EthernetUDP Udp, int sendSize);
    void writeHeader(int type, int blockNum, int payloadLength);
    void writePayload(const char* payloadStartIndex, int payloadLength, uint8_t resultLength);
    void readPayload(int op_index);
    void resetVariables();
    char* buffer;
    bool writePayloadAllowed;
    bool writeResultsAllowed;
    bool error;
    // Function Pointers
    void (*opRefs[10])(uint8_t, uint8_t[6], JsonObject&, uint8_t[6]);
    char* opLabels[10];
    uint8_t opRefIndex ;
    uint8_t results[10];
    void addOperationFunction(void (*foo)(uint8_t, uint8_t[6], JsonObject&, uint8_t[6]), char* id);
    void retrieveResults(JsonObject& root, int op_index);
    bool newOperation;
  private:
    void parseChunk(int op_index);
    void writeOption(int optNum, int len, char* content);
    boolean readOptionDesc();
    void writeCode(char code);
    void writeType(char type);
};

#endif
