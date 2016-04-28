#include <Arduino.h>

#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet2.h>
#include <EthernetUdp2.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include "Semantic.h"
//#include <EEPROM.h>
#include "Coap.h"
#include "Semantic.h"
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
#define PAYLOAD_START B11111111
#define M B00001000
#define PAYLOAD_MAX_SIZE 64
#define OPT_DELTA_EXTENDED B11010000

//////////////////////////////////////////////////////////////
// CAPABILITIES
//////////////////////////////////////////////////////////////


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:

byte mac[] = {
  0x90, 0xA2, 0xDA, 0x10, 0x18, 0x6F
};
IPAddress ip(10, 0, 0, 2);

unsigned int localPort = 5683;      // local port to listen o

char buffer[UDP_TX_PACKET_MAX_SIZE];
// uri and payload correspondance
uripayload_t uripayloads[5];
// URI-Path index and length
String path="/";
// Request
int const_payload_index=-1;
int payloadLen = 0;
// Blocks

//////////////////////////////////////////////////////////////
// EEPROM
//////////////////////////////////////////////////////////////
#define temperature_eeprom_index 0


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
Coap coap= Coap(buffer);



//////////////////////////////////////////////////////////////
// APP LEVEL
//////////////////////////////////////////////////////////////
// get length of a payload in PROGMEM
int getPayloadLength_P(const char* payloadIndex){
  int len =0;
  while(pgm_read_byte(payloadIndex)!='\0'){
    len++;
    payloadIndex++;
  }
  return len;
}

// if the URI corresponds to a unique static payload
// returns the index of such struct uripayload_t
int getUriPayload(int* uri){
  //uri[0] = uri start index in buffer[]
  // uri[1] = uri length
  // if there's no specified path
  Serial.println(uri[1]);
  if(uri[1]==0) path="/";
  else // read the path from the header
    for (int i=0; i<uri[1]; i++)
      path+=buffer[uri[0]+i];
  // check if the path corresponds to a PROGMEM payload
  for (int i=0; i<10; i++)
    if (path.equals(uripayloads[i].path))
      return i;
  return -1;
}

void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.begin(9600);
  // assignment of uris to the corresponding payloads
  uripayloads[0].path="/";
  uripayloads[0].const_payload=CAPABILITIES;
  uripayloads[0].variables=0;
  uripayloads[1].path="/temperature";
  uripayloads[1].const_payload=TEMPERATURE;
  uripayloads[1].variables=2;
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    //load the buffer with the incomming request
    Udp.read(buffer, UDP_TX_PACKET_MAX_SIZE);
    int blockValue = coap.parseHeader();
    Serial.println(blockValue);
    // if it's the first block
    if(((255&blockValue)>>4)==0){
      //TODO: Parse the URI to know what to do with it
      // get the payload index in progmem
      // for current request
      const_payload_index = getUriPayload(coap.uri);
      payloadLen = getPayloadLength_P(uripayloads[const_payload_index].const_payload);
      coap.request="";
    }
    // *** NEVER DO THIS ***
    //Serial.println(uripayloads[const_payload_index].path);
    coap.readPayload();
    // *** treat GET with block2 ***
    // no payload in the request!
    if(coap.method==1) {
      // get the payload for the current uri
      coap.writeHeader(ACK,2, VALID,(255&blockValue)>>4, payloadLen);
      coap.writePayload(uripayloads[const_payload_index].const_payload, payloadLen);
    }
    // *** treat POST with block1 ***
    // payload in the request
    if(coap.method==2) {
      //if no More blocks
      if((blockValue&M)==0){
        // TODO:
        // Do something with the received payload
        Serial.println(coap.request);
      }
      coap.writeHeader(ACK,1, CHANGED,(255&blockValue)>>4, 0);
    }
    coap.sendBuffer(Udp, coap.packetCursor);
    // *** reset global variables ***

    coap.resetVariables();
    delay(10);
  }
}
