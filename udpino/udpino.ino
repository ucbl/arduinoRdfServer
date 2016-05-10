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

int getPayloadLength_P(const char* payloadIndex){
  int len =0;
  while(pgm_read_byte(payloadIndex)!='\0'){
    len++;
    payloadIndex++;
  }
  return len;
}

EthernetUDP Udp;
Coap coap= Coap(buffer);
ResourceManager rsm;

void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.begin(9600);
  rsm.resetMemory();
  // read resources already in EEPROM
  rsm.initialize();
  //TODO: ressources initialized automatically upon reading JSON files
  rsm.addResource(A0,"temperature",TEMPERATURE);
  rsm.addResource(13,"light",LIGHT);
  rsm.printResources();
  //TODO: better way to retrieve resources
  //TODO: parsing capabilities should be called followed by addOperation
  char *temperatureSense = "tempSense";
  char *lightSwitch = "lightSwitch";
  //(uri, method, expects_index, returns_index);
  rsm.addOperation(&(*temperatureSense),0,-1,rsm.idInUse("temperature"));
  rsm.addOperation(&(*lightSwitch),1,rsm.idInUse("light"),-1);
  rsm.printOperations();
  pinMode(rsm.resources[rsm.operations[op_index].expects_index].pin, OUTPUT);
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
      //TODO. retrieve the path in a cleaner fashion
      if(coap.uri[1]!=0){
        for(uint8_t i=0;i<EEPROM_RESOURCE_ALLOC_SIZE;i++){
          if(i<coap.uri[1])
            path[i]=coap.buffer[coap.uri[0]+i];
          else
            path[i]='\0';
        }
      } else {
        for(uint8_t i=0;i<EEPROM_RESOURCE_ALLOC_SIZE;i++){
          if(i==0)
            path[i]='/';
          else
            path[i]='\0';
        }
      }
      Serial.print(F("Parsed URI from CoAP request: "));
      Serial.println(path);
      op_index = rsm.uriInUse(path);
      if(op_index>=0){
        Serial.println(F("Corresponding operation found!"));
        payloadLen = getPayloadLength_P(rsm.resources[rsm.operations[op_index].returns_index].json);
        Serial.println(rsm.operations[op_index].returns_index);
        Serial.print(F("PL length: "));
        Serial.println(payloadLen);
        method = rsm.operations[op_index].method;
      } else {
        Serial.println(F("No corresponding operation found, redirecting to '/'"));
        payloadLen = getPayloadLength_P(CAPABILITIES);
        Serial.print(F("PL length: "));
        Serial.println(payloadLen);
        method = 0;
      }

    }
    coap.readPayload();


      if(coap.method==1) {
        coap.writeHeader(ACK,2, VALID,(255&blockValue)>>4, payloadLen);
        if(op_index>=0){
          String data = String(int(analogRead(rsm.resources[rsm.operations[op_index].returns_index].pin)));
          //TODO: treat data separately
          coap.writePayload(rsm.resources[rsm.operations[op_index].returns_index].json, payloadLen, data);
        } else {
          coap.writePayload(CAPABILITIES, payloadLen, "");
        }
      }
      // *** treat POST with block1 ***
      // payload in the request
      if(coap.method==2) {
        if((blockValue&M)==0){
          //if no More blocks
          /* //Parsing the payload example
          StaticJsonBuffer<200> postBuff;
          JsonObject& root = postBuff.parseObject(coap.payload_chunk);
          uint8_t value=0;
          if(root.success()){
              value = root["value"];
              Serial.print(F("Got light value: "));
              Serial.println(value);
          }
          uint8_t pin = rsm.resources[rsm.operations[op_index].expects_index].pin;
          Serial.print(F("Current Pin: "));
          Serial.println(pin);
          if(pin<14){
            if(value == 0)
              digitalWrite(pin, LOW);
            else
              digitalWrite(pin, HIGH);
          }*/
        }
        coap.writeHeader(ACK,1, CHANGED,(255&blockValue)>>4, 0);
      }
      coap.sendBuffer(Udp, coap.packetCursor);
      coap.resetVariables();
      delay(10);
    }

}
