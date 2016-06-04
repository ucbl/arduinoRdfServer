#include <Arduino.h>
#include "Coap.h"

Coap::Coap(char* payloadBuffer, ResourceManager* rsm_p){
  blockType = 0;
  buffer = payloadBuffer;
  payloadCursor=0;
  payload_chunk='{';
  payload_depth=0;
  rsm = rsm_p;
  error =
  writeResultsAllowed =
  newOperation= false;
  resetVariables();
}

void Coap::resetVariables(){
  writePayloadAllowed=false;
  packetCursor=4;
  payloadStartIndex_r=
  uri[0]=
  uri[1]=
  blockIndex=
  optDelta=
  tokenLength=
  opRefIndex = 0;
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
    if(optNumber == 23) blockType=2;
    if(optNumber == 27) blockType=1;
  }

  // 13:  An 8-bit unsigned integer follows the initial byte and
  // indicates the Option Delta minus 13.


  packetCursor+=int(optLength)+1;
  optDelta=optNumber;
  // return whether there are more options
  if(((255&buffer[packetCursor])!=PAYLOAD_START) && ((255&buffer[packetCursor])!=EMPTY)) return true;
  else return false;
}

void Coap::readPayload(int op_index){
  //initialize max payload depth at the beginning of the payload only
  if (payloadStartIndex_r!=0 && rsm->operations[op_index].expects_index>=0){
    int i=payloadStartIndex_r;
    payload_chunk="{";
    while(i<UDP_TX_PACKET_MAX_SIZE && buffer[i]!=EMPTY){
      //if a new { opens, parse what was left behind at the same level
      if(buffer[i]=='{' || buffer[i]=='}') {
        if(payload_depth!=0 ) parseChunk(op_index);
        if(buffer[i]=='{') payload_depth++;
        if(buffer[i]=='}') payload_depth--;
      }
      else {
        payload_chunk+=buffer[i];
      }
      if(payload_depth<0)
        Serial.println(F("error reading payload depth"));
      i++;
    }
  } else {
    Serial.println(F("no payload to parse"));
    StaticJsonBuffer<10> nullbuffer;
    JsonObject& null_obj = nullbuffer.createObject();
    retrieveResults(null_obj, op_index);
  }
}

void Coap::parseChunk(int op_index){
  Serial.println(F("parsing received payload"));
  if(payload_chunk.lastIndexOf(':')>=0){
    if(payload_chunk.lastIndexOf('[')>payload_chunk.lastIndexOf(':')
        && payload_chunk.lastIndexOf(']')<payload_chunk.lastIndexOf('['))
      payload_chunk+=']';
    payload_chunk+='}';
    //once a chunk is complete, treat it with ArduinoJson
    StaticJsonBuffer<200> jsonBuff;
    JsonObject& root = jsonBuff.parse(payload_chunk);
    if(!root.success()){
      Serial.print(F("error parsing: "));
      Serial.println(payload_chunk);
    } else {
      retrieveResults(root, op_index);
    }
  }
  payload_chunk="{";
}

void Coap::retrieveResults(JsonObject& root, int op_index){
  for(uint8_t i=0;i<10;i++){
    if(strcmp(rsm->operations[op_index].uri,opLabels[i])==0 && rsm->operations[op_index].user_defined){
      opRefs[i](
        rsm->operations[rsm->operationInUse(opLabels[i])].pin_count,
        rsm->operations[rsm->operationInUse(opLabels[i])].pins,
        root,
        results);
    }
  }
  if(strcmp_P(rsm->operations[op_index].uri,PSTR("eepromReset"))==0){
    Serial.println(F("--calling eepromReset"));
    for(uint8_t i=0;i<rsm->operation_count;i++){
      if(rsm->operations[i].user_defined){
        memset(rsm->operations[i].uri, '\0', EEPROM_RESOURCE_ALLOC_SIZE);
        memset(rsm->operations[i].pins, NULL, 6);
        rsm->operations[i].pin_count = 0;
      }
    }
    rsm->refreshEeprom();
    rsm->initialize_op();
  } else if(strcmp_P(rsm->operations[op_index].uri,PSTR("eepromAdd"))==0){
    Serial.println(F("--calling eepromAdd"));
    if(rsm->operationInUse((const char *)root["uri"])<0){
      JsonArray& json_pins = root["pins"].asArray();
      rsm->addEepromEntry((int)root["pin_count"], json_pins,(const char *)root["uri"]);
      rsm->initialize_op();
      newOperation = true;
    }
  }
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

void Coap::writeHeader(int type, int blockValue, int payloadLength){
  int blockNum = (255&blockValue)>>4;
  writeType(type);
  if(blockType==1) writeCode(CHANGED);
  else if(blockType==2) writeCode(VALID);
  else writeCode(VALID);
  //no need to change token length or MessageID
  packetCursor=4+tokenLength;
  int more=1;
  if(blockNum>=payloadLength/PAYLOAD_MAX_SIZE)
    more=0;
  char content1[]={B00000010|(blockNum<<4)|(more<<3)};
  char content2[]={B00000010|(blockNum<<4)};
  char firstBlock2[]={B00000010};
  char firstBlock2M[]={B00001010};

  if(blockType==2) {
    writeOption(23,1,content1);
    writePayloadAllowed=true;
  } else if(blockType==1) {
    if((blockValue&M)==0){
      Serial.println(F("Finished uploading, writing payload"));
      if(payloadLength>PAYLOAD_MAX_SIZE)
        writeOption(23,1,firstBlock2M);
      else
        writeOption(23,1,firstBlock2);
      writePayloadAllowed=true;
      payloadCursor=0;
    }
    writeOption(27,1,content2);
  } else if(payloadLength>0){
    Serial.println(F("Finished uploading, writing payload"));
    if(payloadLength>PAYLOAD_MAX_SIZE)
      writeOption(23,1,firstBlock2M);
    else
      writeOption(23,1,firstBlock2);
    writePayloadAllowed=true;
    payloadCursor=0;
  }
}

void Coap::writePayload(const char* payloadStartIndex_w, int payloadLength, uint8_t resultsLength){
  buffer[packetCursor]=PAYLOAD_START;
  packetCursor++;
  //payloadCursor reads from PROGMEM
  //packetCursor writes on buffer
  uint8_t i=0;
  char write_char;
  while (packetCursor<UDP_TX_PACKET_MAX_SIZE && payloadCursor<payloadLength && i<PAYLOAD_MAX_SIZE){
    write_char = pgm_read_byte(payloadStartIndex_w+payloadCursor);
    if(write_char=='\"' && pgm_read_byte(payloadStartIndex_w+payloadCursor+1)=='\"'){
      for(uint8_t j=0;j<resultsLength;j++){
        if(results[j]!=NULL){
          char s_data[3] = {0};
          itoa(results[j], s_data, 10);
          strncat(buffer, s_data, strlen(s_data));
          results[j]=NULL;
          packetCursor+=strlen(s_data);
          i+=strlen(s_data);
          j=resultsLength;
        }
      }
      payloadCursor++;
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
    error = false;
  }
}

void Coap::addOperationFunction(void (*foo)(uint8_t, uint8_t[6], JsonObject&, uint8_t[6]), char* id) {
  opRefs[opRefIndex]=foo;
  opLabels[opRefIndex]=id;
  Serial.print(F("Referencable function added: "));
  Serial.println(id);
  opRefIndex++;
}
