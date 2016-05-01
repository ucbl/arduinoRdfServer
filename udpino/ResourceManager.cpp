#include <Arduino.h>
#include "ResourceManager.h"
#include "Semantic.h"

ResourceManager::ResourceManager(){

}

void ResourceManager::initialize(){
  //checks the ressources stored in eeprom and loads
  //the ressources object with them
  resource_count=0;
  operation_count=0;
  int resource_id_cursor=0;
  bool on_value=false;
  uint8_t val = 0;
  last_index=-1;
  // variables are separated by a NULL space 0
  for (uint16_t i=0;i<1024;i++) {
    val = EEPROM.read(i);
    if(val!=0){
      if(resource_count<EEPROM_RESOURCE_NUM && resource_id_cursor<EEPROM_RESOURCE_ALLOC_SIZE){
        if(on_value) {
          resources[resource_count].id[resource_id_cursor]=char(val);
          resource_id_cursor++;
        }
        else resources[resource_count].pin=val;
      }
      on_value=true;
    } else {
      //count the resource only when it ends
      if(on_value==true){
        Serial.print(F("Separator at: "));
        Serial.println(i);
        on_value=false;
        while(resource_id_cursor<EEPROM_RESOURCE_ALLOC_SIZE){
          resources[resource_count].id[resource_id_cursor]=0;
          resource_id_cursor++;
        }
        resource_count++;
        resource_id_cursor=0;
        //update the last WRITABLE index
        last_index=i;
      }
    }
  }
  Serial.print(resource_count);
  Serial.println(F(" resource(s) currently in EEPROM"));
  Serial.print(F("Next Resource will start at: "));
  Serial.println(last_index+1);
}

void ResourceManager::addResource(uint8_t pin, char* id, const char* json){
  if(pinInUse(pin) || idInUse(id)>=0){
    //not allowed to add the same uri/pin
    Serial.print(F("Can't add "));
    Serial.println(id);
  } else {
    Serial.print(F("Adding: "));
    Serial.print(pin);
    Serial.println(id);
    if(resource_count<EEPROM_RESOURCE_NUM)
      resources[resource_count].pin=pin;
    for (unsigned int i=0;i<strlen(id);i++){
      if((resource_count<EEPROM_RESOURCE_NUM) && (i<EEPROM_RESOURCE_ALLOC_SIZE))
        resources[resource_count].id[i]=id[i];
    }
    resources[resource_count].json=json;
    //add to eeprom
    EEPROM.write(last_index+1,resources[resource_count].pin);
    for (unsigned int i=0; i<strlen(id);i++){
      EEPROM.write(last_index+2+i,resources[resource_count].id[i]);
    }
    last_index+=strlen(id)+2;
    resource_count++;
    Serial.print(F("Number of resources "));
    Serial.println(resource_count);
  }
}

void ResourceManager::addOperation(char* uri, uint8_t method, int expects_index, int returns_index){
  if(uriInUse(uri)<0){
    operation_t op;
    strcpy(op.uri,uri);
    Serial.print(F("operation added: "));
    Serial.println(op.uri);
    op.method = method;
    op.expects_index = expects_index;
    op.returns_index = returns_index;
    operations[operation_count]=op;
    operation_count++;
  }
}

void ResourceManager::resetMemory(){
  for (uint16_t i=0;i<1024;i++)
    EEPROM.write(i,0);
}

void ResourceManager::printResources(){
  for(unsigned int i=0;i<resource_count;i++){
    Serial.print(resources[i].pin);
    Serial.print(F(","));
    Serial.println(resources[i].id);
  }
}

void ResourceManager::printOperations(){
  for(unsigned int i=0;i<operation_count;i++){
    Serial.print(operations[i].uri);
    Serial.print(F(","));
    Serial.println(operations[i].method);
  }
}

void ResourceManager::setPin(uint8_t pin, char* id){
  for(unsigned int i=0;i<resource_count;i++){
    if(strcmp(id,resources[i].id)==0 && !pinInUse(pin)){
      resources[i].pin=pin;
      return;
    } else {
      //Serial.print(F("SetPin not allowed for "));
      //Serial.println(resources[i].id);
    }
  }
}

void ResourceManager::parseCapabilities(const char* capabilities){
  /*
  StaticJsonBuffer<600> jsonBuffer;
  char capabilities_s[600];
  unsigned int c=0;
  int depth=0;
  boolean eoc= false;
  char* cpb_start_ptr = (char*)strchrnul_P(capabilities, int('['));
  while(!eoc && c<600){
    capabilities_s[c]=pgm_read_byte(cpb_start_ptr+c);
    if(capabilities_s[c]=='[')
      depth++;
    if(capabilities_s[c]==']')
      depth--;
    if(depth == 0) {
      //finished the chunk, mark the index
      eoc=true;
    }
    c++;
  }
  JsonArray& cpb_root = jsonBuffer.parseArray(capabilities_s);
  JsonObject& cpb= cpb_root.get(0);
  JsonArray& supportedOperation= cpb["supportedOperation"];
  JsonObject& op1= supportedOperation.get(0);
  operation_t op;
  strcpy(op.uri,op1["@id"]);
  if(op1["method"]=="GET") op.method=0;
  if(op1["method"]=="POST") op.method=1;
  strcpy(op.expects, op1["expects"]);
  strcpy(op.returns, op1["returns"]);
  addOperation(op);
  */
}

int ResourceManager::idInUse(char* id){
  // make sure the id can be compared
  /*char full_id[EEPROM_RESOURCE_ALLOC_SIZE];
  for(uint8_t i=0;i<EEPROM_RESOURCE_ALLOC_SIZE;i++){
    if(i<strlen(id)) full_id[i]=id[i];
    else full_id[i]=char(0);
  }*/
  for(unsigned int i=0;i<resource_count;i++){
    //if(strcmp(resources[i].id,full_id)==0)
    if(strcmp(resources[i].id,id)==0)
      return i;
  }
  return -1;
}

int ResourceManager::uriInUse(char *uri){
  // uri has to have the good size
  // assume in use, return if not verified to be different
  boolean inUse;
  for(uint8_t i=0;i<operation_count;i++){
    inUse = true;
    for(uint8_t j=0;j<EEPROM_RESOURCE_ALLOC_SIZE;j++)
      if(uri[j]!='\0' && operations[i].uri[j]!=uri[j]){
        inUse=false;
      }
    if(inUse) return i;
  }
  return -1;
}


boolean ResourceManager::pinInUse(uint8_t pin){
  for(unsigned int i=0;i<resource_count;i++){
    if(resources[i].pin==pin) return true;
  }
  return false;
}
