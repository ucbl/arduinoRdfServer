#ifndef Coap_h
#define Coap_h

#include <Arduino.h>
#include <EthernetUdp2.h>

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
    Coap(char* payloadBuffer);
    int parseHeader();

    int packetCursor;
    int payloadCursor;
    int tokenLength;
    int uri[2];
    String path;
    int optDelta;
    // Request
    int method;
    int payloadStartIndex_r;
    String request;
    int const_payload_index;
    int payloadLen;
    // Blocks
    int blockIndex;
    void sendBuffer(EthernetUDP Udp, int sendSize);
    void writeHeader(int type, int block, int code, int blockNum, int payloadLength);
    void writePayload(const char* payloadStartIndex, int payloadLength, char variable);
    void readPayload();
    void resetVariables();
  private:
    int variable_cursor;
    char* buffer;
    void writeOption(int optNum, int len, char* content);
    boolean readOptionDesc();
    void writeCode(char code);
    void writeType(char type);
};

#endif