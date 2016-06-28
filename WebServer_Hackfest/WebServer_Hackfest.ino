/*
  Semantic Web Server

  Simple server capable of linking sensor functions to REST services through JSON-LD annotation.

  In this example : Light Switch and Temperature Sense functions can be enabled, configured and querried.
  Different setups can be arranged through REST calls without the need of flashing new software.

  Sensor level functions are decoupled from the server, so that users are encouraged to modify them according to their needs.

  New functions can be added at compilation time as long as users provide the information in the JSON-LD EntryPoint.

 ArduinoJson library by Benoît Blanchon
 https://github.com/bblanchon/ArduinoJson

    Lionel Médini, June 2015
    Remy Rojas, March 2016

*/
#include <Arduino.h>

#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet2.h>
#include <EthernetUdp2.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <EEPROM.h>
#include "Coap.h"
#include "Semantic.h"
#include "ResourceManager.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:

byte mac[] = {
  0x90, 0xA2, 0xDA, 0x10, 0x18, 0x6F
};
IPAddress ip(10, 0, 0, 2);

unsigned int localPort = 5683;      // local port to listen o

char buffer[UDP_TX_PACKET_MAX_SIZE];

int op_index=-1;
char path[EEPROM_RESOURCE_ALLOC_SIZE];
int payloadLen = 0;
uint8_t method = 0;
/****************************************/
//Function pointing
uint8_t opRefIndex = 0;

void getPath(Coap* coap){
  if(coap->uri[1]!=0){
    for(uint8_t i=0;i<EEPROM_RESOURCE_ALLOC_SIZE;i++){
      if(i<coap->uri[1])
        path[i]=coap->buffer[coap->uri[0]+i];
      else
        path[i]='\0';
    }
  } else {
    for(uint8_t i=0;i<EEPROM_RESOURCE_ALLOC_SIZE;i++){
        path[i]='\0';
    }
  }
}

int getPayloadLength_P(const char* payloadIndex){
  int len =0;
  while(pgm_read_byte(payloadIndex)!='\0'){
    len++;
    payloadIndex++;
  }
  return len;
}

EthernetUDP Udp;
ResourceManager rsm;
Coap coap= Coap(buffer, &rsm);

void setup() {
  // *******************************
  // link functions in Semantic.cpp with JSON-LD descriptions
  coap.addOperationFunction(&tempSense_f, (char* )"tempSense");
  coap.addOperationFunction(&lightSwitch_f, (char* )"lightSwitch");
  // ******************************
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.begin(9600);
  rsm.initialize_re();
  rsm.printResources();
  rsm.initialize_op();
  rsm.parseCapabilities(CAPABILITIES, &coap.payload_chunk);
  rsm.printOperations();
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
    if(((255&blockValue)>>4)==0 && coap.payloadCursor==0){
      coap.writeResultsAllowed=true;
      coap.payloadCursor = 0;
      getPath(&coap);
      op_index = rsm.operationInUse(path);
      if(op_index>=0){
        if(rsm.operations[op_index].returns_index>=0)
          payloadLen = getPayloadLength_P(rsm.resources[rsm.operations[op_index].returns_index].json);
        else{
          payloadLen=0;
        }
        Serial.println(payloadLen);
        method = rsm.operations[op_index].method;
        Serial.println(method);
        Serial.print(F("Returns: "));
        Serial.println(rsm.resources[rsm.operations[op_index].returns_index].id);
      } else {
        coap.error = true;
        Serial.println(F("Error"));
      }
      if(rsm.operations[op_index].expects_index<0)
        coap.writeResultsAllowed=true;
    }
    // look for errors
    if(coap.method!=rsm.operations[op_index].method ||
        coap.results[5]==1)
      coap.error=true;
    coap.readPayload(op_index);
    if(coap.newOperation && coap.method!=1){
      rsm.parseCapabilities(CAPABILITIES, &coap.payload_chunk);
      rsm.printOperations();
      coap.newOperation=false;
    }
    // if no errors proceed normally, otherwise return error
    if(!coap.error){
      coap.writeHeader(ACK, blockValue, payloadLen);
      if(coap.writePayloadAllowed)
        coap.writePayload(rsm.resources[rsm.operations[op_index].returns_index].json, payloadLen, rsm.operations[op_index].pin_count);
    }
    else {
      coap.writeHeader(ACK, blockValue, getPayloadLength_P(ERROR));
      coap.writePayload(ERROR, getPayloadLength_P(ERROR), 0);
    }

    coap.sendBuffer(Udp, coap.packetCursor);
    coap.resetVariables();
    delay(10);
  }

}
