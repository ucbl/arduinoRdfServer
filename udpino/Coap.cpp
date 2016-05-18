#include <Arduino.h>
#include "Coap.h"

Coap::Coap(char* payloadBuffer){
  buffer = payloadBuffer;
  payloadCursor=0;
  payload_chunk="{";
  payload_depth=0;
  resetVariables();
}

void Coap::resetVariables(){
  packetCursor=4;
  payloadStartIndex_r=
  uri[0]=
  uri[1]=
  blockIndex=
  optDelta=
  tokenLength=0;
  memset(buffer,0,UDP_TX_PACKET_MAX_SIZE);
}

void Coap::sendBuffer(EthernetUDP Udp, int sendSize){
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(buffer,sendSize);
  Udp.endPacket();
}

void Coap::writeCode(char code){
  buffer[1]&=B00000000;
  buffer[1]|=code;
}

void Coap::writeType(char type){
  buffer[0]&=B11001111;
  buffer[0]|=type;
}

boolean Coap::readOptionDesc(){
  int optNumber=((255&buffer[packetCursor])>>4)+optDelta;
  //extended delta
  if (((255&buffer[packetCursor])>>4)==13) optNumber+=int(255&buffer[packetCursor+1]);
  int optLength=int(buffer[packetCursor]&B00001111);
  // 11: URI-Path
  if(int(optNumber)==11){
    uri[0]=packetCursor+1;
    if(int(optLength)==13){
      optLength+=int(255&buffer[packetCursor+1]);
      packetCursor++;
      uri[0]++;
    }
    uri[1]=optLength;
  }
  // *** if BLOCK ***
  // attribute an index to blockIndex
  if(optNumber==23 || optNumber==27) {
    blockIndex=packetCursor;
    if(((255&buffer[packetCursor])>>4)==13){
      packetCursor++;
    }
  }

  // 13:  An 8-bit unsigned integer follows the initial byte and
  // indicates the Option Delta minus 13.


  packetCursor+=int(optLength)+1;
  optDelta=optNumber;
  // return whether there are more options
  if(((255&buffer[packetCursor])!=PAYLOAD_START) && ((255&buffer[packetCursor])!=EMPTY)) return true;
  else return false;
}

void Coap::readPayload(){
  //initialize max payload depth at the beginning of the payload only
  if (payloadStartIndex_r!=0){
    int i=payloadStartIndex_r;
    while(i<UDP_TX_PACKET_MAX_SIZE && buffer[i]!=EMPTY){
      //if a new { opens, parse what was left behind at the same level
      if(buffer[i]=='{' || buffer[i]=='}') {
        if(payload_depth!=0 ) parseChunk();
        if(buffer[i]=='{') payload_depth++;
        if(buffer[i]=='}') payload_depth--;
      }
      else payload_chunk+=buffer[i];
      if(payload_depth<0) Serial.println(F("error reading payload depth"));
      i++;
    }
  }
}

void Coap::parseChunk(){
  if(payload_chunk.indexOf(':')>=0){
    if(payload_chunk.lastIndexOf('[')>payload_chunk.lastIndexOf(':')
        && payload_chunk.lastIndexOf(']')<payload_chunk.lastIndexOf('['))
      payload_chunk+=']';
    payload_chunk+='}';
    //once a chunk is complete, treat it with ArduinoJson
    StaticJsonBuffer<300> jsonBuff;
    JsonObject& root = jsonBuff.parse(payload_chunk);
    if(!root.success()){
      Serial.print(F("error parsing: "));
      Serial.println(payload_chunk);
    }
  }
  payload_chunk="{";
}

int Coap::parseHeader(){
  // *** store packet into buffer ***
  // tkl offset
  tokenLength=(15&buffer[0]);
  packetCursor=4+tokenLength;
  // check and read options
  // exit when no more options
  if((buffer[packetCursor]!=PAYLOAD_START) && (buffer[packetCursor]!=EMPTY)){
    boolean options = true;
    while(options){
      options = readOptionDesc();
    }
  }
  // *** check if payload ***
  if((255&buffer[packetCursor])==PAYLOAD_START){
    payloadStartIndex_r=packetCursor+1;
  }
  method = uint8_t(buffer[1]);
  optDelta=0;

  //return the block value if any
  //otherwise return 0
  if(blockIndex!=0){
    if((buffer[blockIndex]&1)!=0){
      if(((255&buffer[blockIndex])>>4)==13) return int(buffer[blockIndex+2]);
      else return int(buffer[blockIndex+1]);
    }
  }
  return 0;
}

void Coap::writeOption(int optNum, int len, char* content){
  // write the length option's length
  if(len<16)  buffer[packetCursor]=EMPTY|len;
  // write the option's number
  if(optNum-optDelta>12){
    buffer[packetCursor]|=OPT_DELTA_EXTENDED;
    if(optNum-13-optDelta<16)  buffer[packetCursor+1]=optNum-13;
    packetCursor+=2;
  } else {
    buffer[packetCursor]|=(optNum-optDelta)<<4;
    packetCursor++;
  }
  // write the option's
  for(int i=0;i<len;i++) {
    buffer[packetCursor]=content[i];
    packetCursor++;
  }
}

void Coap::writeHeader(int type, int block, int code, int blockNum, int payloadLength){
  writeType(type);
  writeCode(code);
  //no need to change token length or MessageID
  packetCursor=4+tokenLength;
  //TODO: A more intelligent way to treat block responses
  int more=1;
  if(blockNum>=payloadLength/PAYLOAD_MAX_SIZE) more=0;
  char content1[]={B00000010|(blockNum<<4)|(more<<3)};
  char content2[]={B00000010|(blockNum<<4)};

  if(block==2) writeOption(23,1,content1);
  if(block==1) writeOption(27,1,content2);
}

void Coap::writePayload(const char* payloadStartIndex_w, int payloadLength, String data){
  buffer[packetCursor]=PAYLOAD_START;
  packetCursor++;
  //payloadCursor reads from PROGMEM
  //packetCursor writes on buffer
  uint8_t variable_cursor= 0;
  uint8_t i=0;
  char write_char;
  while (packetCursor<UDP_TX_PACKET_MAX_SIZE && payloadCursor<payloadLength && i<PAYLOAD_MAX_SIZE){
    write_char = pgm_read_byte(payloadStartIndex_w+payloadCursor);
    if(write_char=='#' && data){
      for(uint8_t j=0;j<data.length();j++){
        buffer[packetCursor]=data[j];
        variable_cursor++;
        packetCursor++;
        i++;
      }
      Serial.println();
    } else {
      buffer[packetCursor]=write_char;
      packetCursor++;
      i++;
    }
    payloadCursor++;
  }
  //once the last block of payload is written
  if(payloadCursor>=payloadLength) {
    payloadCursor = 0;
  }
}
