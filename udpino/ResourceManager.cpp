#include <Arduino.h>
#include "ResourceManager.h"
#include "Semantic.h"

ResourceManager::ResourceManager(){
  operation_count=0;
  eeprom_cursor=-1;
}

void ResourceManager::initialize_op(){
  //checks the variables stored in eeprom and loads
  //the operations object with them

  operation_count=0;
  bool on_value=false;
  uint8_t val = 0x00;
  eeprom_cursor=0;
  // operations are separated by a NULL space 0
  for (int i=0;i<1024;i++) {
    val = EEPROM.read(i);
    //if not separator
    if(val!=0x00 && operation_count<EEPROM_RESOURCE_NUM){
      //on_value is false then we're on the first byte after separator
      if(on_value) {
        //check if we're reading pins or uri
        if(i-eeprom_cursor<operations[operation_count].pin_count){
          operations[operation_count].pins[i-eeprom_cursor]=val;
        } else {
          operations[operation_count].uri[i-operations[operation_count].pin_count-eeprom_cursor]=val;
        }
      } else {
        operations[operation_count].pin_count=val;
        eeprom_cursor++;
      }
      on_value=true;
    //if separator
    } else {
      //when we find a first NULL byte the message has ended
      //A.K.A. do once
      if(on_value==true){
        //fill the uri properly
        for(uint8_t j=i-operations[operation_count].pin_count-eeprom_cursor;j<EEPROM_RESOURCE_ALLOC_SIZE;j++)
          operations[operation_count].uri[j]='\0';
        on_value=false;
        operation_count++;
        eeprom_cursor=i+1;
      }
      //place the cursor where a new entry WOULD be
    }
  }
  Serial.print(operation_count);
  Serial.println(F(" operations currently in EEPROM"));
  addOperation("", "GET", -1, resourceInUse("capabilities"));
}

void ResourceManager::initialize_re(){
  for(uint8_t i=0;i<RESOURCE_COUNT;i++){
    String buff = "{";
    int j=1;
    char c=pgm_read_byte(pgm_read_word(&(json_resources[i]))+j);
    while(c!='{' && c!='}'){
      buff+=c;
      j++;
      c=pgm_read_byte(pgm_read_word(&(json_resources[i]))+j);
    }
    buff+='}';
    Serial.println(buff);
    StaticJsonBuffer<200> jsonBuff;
    JsonObject& root = jsonBuff.parse(buff);
    if(root.success()){
      addResource(strdup((const char*)root["@type"]),(const char*)pgm_read_word(&(json_resources[i])), i);
    } else {
      Serial.println(F("Error adding resource"));
    }
  }
  addResource("capabilities", CAPABILITIES, RESOURCE_COUNT);
}

void ResourceManager::addResource(char* id, const char* json, uint8_t index){
  if(resourceInUse(id)>=0){
    //not allowed to add the same id twice
    Serial.print(F("Can't add "));
    Serial.println(id);
  } else {
    for (uint8_t i=0;i<EEPROM_RESOURCE_ALLOC_SIZE;i++){
      if(i<strlen(id))
        resources[index].id[i]=id[i];
      else
        resources[index].id[i]='\0';
    }
    resources[index].json=json;
  }
}

void ResourceManager::addEepromEntry(uint8_t pin_count, uint8_t* pins, char* uri){
  //add information to eeprom
  //if it is in eeprom already it would be also in memory (initialize_op)
  //so check in memory
  if(operationInUse(uri)<0){
    EEPROM.write(eeprom_cursor,pin_count);
    eeprom_cursor++;
    for (uint8_t i=0;i<pin_count;i++)
      EEPROM.write(eeprom_cursor+i,pins[i]);
    eeprom_cursor+=pin_count;
    int j=0;
    while(uri[j]!='\0' && j<EEPROM_RESOURCE_ALLOC_SIZE && j<strlen(uri)){
      EEPROM.write(j+eeprom_cursor,uri[j]);
      j++;
    }
    eeprom_cursor+= j+1;
    //add information to existing operation_t list
    operations[operation_count].pin_count=pin_count;
    for(uint8_t i=0;i<pin_count;i++)
      operations[operation_count].pins[i] = pins[i];
    for(uint8_t i=0;i<EEPROM_RESOURCE_ALLOC_SIZE;i++){
      if(i<strlen(uri))
        operations[operation_count].uri[i] = uri[i];
      else
        operations[operation_count].uri[i] = '\0';
    }
    operation_count++;
  } else {
    Serial.print(F("Operation \""));
    Serial.print(uri);
    Serial.println(F("\" already in use"));
  }
}

void ResourceManager::addOperation(char* uri, char* method, int expects_index, int returns_index){
  for(uint8_t i=0;i<EEPROM_RESOURCE_ALLOC_SIZE;i++){
    if(i<strlen(uri))
      operations[operation_count].uri[i] = uri[i];
    else
      operations[operation_count].uri[i] = '\0';
  }
  completeOperation(operation_count, method, expects_index, returns_index);
  operation_count++;
}

void ResourceManager::completeOperation(int operation_index,char* method, int expects_index, int returns_index){
  if(strcmp_P(method,PSTR("GET"))==0)
    operations[operation_index].method = 0;
  if(strcmp_P(method,PSTR("POST"))==0)
    operations[operation_index].method = 1;
  operations[operation_index].expects_index = expects_index;
  operations[operation_index].returns_index = returns_index;
}

void ResourceManager::resetMemory(){
  for (uint16_t i=0;i<1024;i++)
    EEPROM.write(i,0x00);
}

void ResourceManager::printResources(){
  Serial.println(F("----------Resources in RAM:"));
  for(unsigned int i=0;i<RESOURCE_COUNT+1;i++){
    Serial.println(resources[i].id);
  }
}

void ResourceManager::printOperations(){
  Serial.print(F("----------"));
  Serial.print(operation_count);
  Serial.println(F(" Operations in Memory"));
  for(unsigned int i=0;i<operation_count;i++){
    Serial.print(F("uri, pin_count, method: "));
    Serial.print(operations[i].uri);
    Serial.print(F(", "));
    Serial.print(operations[i].pin_count);
    Serial.print(F(", "));
    Serial.println(operations[i].method);
  }
}

void ResourceManager::setPin(uint8_t pin_count, uint8_t* pins, char* uri){
  int operation_index=operationInUse(uri);
  //operation exists and pin number is equal
  if(operation_index>=0 && pin_count==operations[operation_index].pin_count){
    for(uint8_t i=0;i<pin_count;i++){
      operations[operation_index].pins[i]=pins[i];
    }
  }
}

void ResourceManager::parseCapabilities(const char* json, String* chunk){
  int json_depth = 0;
  char c='c';
  int i = 0;
  *chunk="{";
  while(c!='\0'){
    c=pgm_read_byte(json+i);
    if(c=='{' || c=='}'){
      if(json_depth>0) parseChunk(chunk);
      if(c=='{') json_depth++;
      if(c=='}') json_depth--;
      *chunk="{";
    }
    else *chunk+=c;
    i++;
  }
}

void ResourceManager::parseChunk(String* chunk){
  if((*chunk).indexOf(':')>=0){
    if((*chunk).lastIndexOf('[')>(*chunk).lastIndexOf(':')
        && (*chunk).lastIndexOf(']')<(*chunk).lastIndexOf('['))
      (*chunk)+=']';
    (*chunk)+='}';
    //once a chunk is complete, treat it with ArduinoJson
    StaticJsonBuffer<300> jsonBuff;
    JsonObject& root = jsonBuff.parse(*chunk);
    if(!root.success()){
      Serial.print(F("Error parsing: "));
      Serial.println(*chunk);
    }
    if(strcmp_P((const char*)root["@type"],PSTR("hydra:Operation"))==0){
      Serial.print(F("Complementing information for Operation labeled "));
      Serial.println((const char*)root["label"]);
      int operation_index=operationInUse(strdup((const char*)root["@id"]));
      if(operation_index>=0){
        completeOperation(
          operation_index,
          strdup((const char*)root["method"]),
          resourceInUse(strdup((const char*)root["expects"])),
          resourceInUse(strdup((const char*)root["returns"]))
        );
      }
    }
  }
}

int ResourceManager::resourceInUse(char* id){
  for(unsigned int i=0;i<RESOURCE_COUNT+1;i++){
    if(strcmp(resources[i].id,id)==0)
      return i;
  }
  return -1;
}

int ResourceManager::operationInUse(char *uri){
  for(uint8_t i=0;i<operation_count;i++)
    if(strcmp(operations[i].uri,uri)==0)
      return i;
  return -1;
}


boolean ResourceManager::pinInUse(uint8_t pin){
  for(unsigned int i=0;i<operation_count;i++){
    for(uint8_t j=0;j<operations[i].pin_count;j++)
      if(operations[i].pins[j]==pin) return true;
  }
  return false;
}
