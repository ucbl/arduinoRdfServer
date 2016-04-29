#include <Arduino.h>

#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet2.h>
#include <EthernetUdp2.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <EEPROM.h>
#include "Coap.h"
#include "Semantic.h"
#include "EepromManager.h"

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
//////////////////////////////////////////////////////////////
// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
Coap coap= Coap(buffer);
EepromManager epm;
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
  if(uri[1]==0) path="/";
  else // read the path from the header
    for (int i=0; i<uri[1]; i++)
      path+=buffer[uri[0]+i];
  // check if the path corresponds to a PROGMEM payload
  for (int i=0; i<sizeof(uripayloads); i++)
    if (path.equals(uripayloads[i].path))
      return i;
  // otherwise return -1
  return 0;
}


// *** Reads the physical ressource for a given Pin ***
char getVariables(){
  for (int i=0;i<epm.ressource_count; i++){
    if(path.compareTo(String(epm.ressources[i].uri))==0){
      Serial.println(F("Request matches a Ressource!"));
      return char(int(analogRead(epm.ressources[i].pin)));
    }
  }
  Serial.println(F("Request did not match any Ressource :("));
  return 'N';
}

void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.begin(9600);
  // assignment of uris to the corresponding payloads
  uripayloads[0].path="/error";
  uripayloads[0].const_payload=ERROR;
  uripayloads[1].path="/";
  uripayloads[1].const_payload=CAPABILITIES;
  uripayloads[2].path="/temperature";
  uripayloads[2].const_payload=TEMPERATURE;
  epm.resetMemory();
  epm.initialize();
  epm.addRessource(3,"/temperature");
  /*epm.initialize();
  epm.addRessource(1,"light");
  epm.initialize();*/
  epm.setPin(2, "/temperature");
  epm.printRessources();

}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    //load the buffer with the incomming request
    Udp.read(buffer, UDP_TX_PACKET_MAX_SIZE);
    int blockValue = coap.parseHeader();
    // if it's the first block
    if(((255&blockValue)>>4)==0){
      //TODO: Parse the URI to know what to do with it
      // get the payload index in progmem
      // for current request
      const_payload_index = getUriPayload(coap.uri);
      payloadLen = getPayloadLength_P(uripayloads[const_payload_index].const_payload);
    }
    Serial.println(uripayloads[const_payload_index].path);
    coap.readPayload();
    // *** treat GET with block2 ***
    // no payload in the request!
    if(coap.method==1) {
      // get the payload for the current uri
      coap.writeHeader(ACK,2, VALID,(255&blockValue)>>4, payloadLen);
      char variable = getVariables();
      coap.writePayload(uripayloads[const_payload_index].const_payload, payloadLen, variable);
    }
    // *** treat POST with block1 ***
    // payload in the request
    if(coap.method==2) {
      //if no More blocks
      if((blockValue&M)==0){
        // TODO:
        // Do something with the received payload
        Serial.println(coap.request);
        coap.request="";
      }
      coap.writeHeader(ACK,1, CHANGED,(255&blockValue)>>4, 0);
    }
    coap.sendBuffer(Udp, coap.packetCursor);
    // *** reset global variables ***

    coap.resetVariables();
    delay(10);
  }
}
