#include <Arduino.h>
#include "ResourceManager.h"
#include "Semantic.h"

ResourceManager::ResourceManager(){

}

void ResourceManager::initialize(){
  //checks the ressources stored in eeprom and loads
  //the ressources object with them
  resource_count=0;
  int resource_uri_cursor=0;
  bool on_value=false;
  uint8_t val = 0;
  last_index=-1;
  // variables are separated by a NULL space 0
  for (uint16_t i=0;i<1024;i++) {
    val = EEPROM.read(i);
    if(val!=0){
      if(resource_count<EEPROM_RESOURCE_NUM && resource_uri_cursor<EEPROM_RESOURCE_ALLOC_SIZE){
        if(on_value) {
          resources[resource_count].uri[resource_uri_cursor]=char(val);
          resource_uri_cursor++;
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
        while(resource_uri_cursor<EEPROM_RESOURCE_ALLOC_SIZE){
          resources[resource_count].uri[resource_uri_cursor]=0;
          resource_uri_cursor++;
        }
        resource_count++;
        resource_uri_cursor=0;
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

void ResourceManager::addResource(uint8_t pin, char* uri){
  if(pinInUse(pin) || uriInUse(uri)){
    //not allowed to add the same uri/pin
    Serial.print(F("Can't add "));
    Serial.println(uri);
  } else {
    //add to the 2D array
    Serial.print(F("Adding: "));
    Serial.print(pin);
    Serial.println(uri);
    if(resource_count<EEPROM_RESOURCE_NUM)
      resources[resource_count].pin=pin;
    for (unsigned int i=0;i<strlen(uri);i++){
      if((resource_count<EEPROM_RESOURCE_NUM) && (i<EEPROM_RESOURCE_ALLOC_SIZE))
        resources[resource_count].uri[i]=uri[i];
    }
    //add to eeprom
    EEPROM.write(last_index+1,resources[resource_count].pin);
    for (unsigned int i=0; i<strlen(uri);i++){
      EEPROM.write(last_index+2+i,resources[resource_count].uri[i]);
    }
    last_index+=strlen(uri)+2;
    resource_count++;
    Serial.print(F("Next separator should be at "));
    Serial.println(last_index);
    Serial.print(F("Number of resources"));
    Serial.println(resource_count);
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
    Serial.println(resources[i].uri);
  }
}

void ResourceManager::setPin(uint8_t pin, char* uri){
  for(unsigned int i=0;i<resource_count;i++){
    if(strcmp(uri,resources[i].uri)==0 && !pinInUse(pin)){
      resources[i].pin=pin;
      return;
    } else {
      Serial.print(F("SetPin not allowed for "));
      Serial.println(resources[i].uri);
    }
  }
}

void ResourceManager::parseCapabilities(const char* capabilities){
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
  const char* label = cpb["label"];
  Serial.println(label);
}
///////////////////////////////////
// PRIVATE
//////////////////////////////////

boolean ResourceManager::uriInUse(char* uri){
  //for every resource, compare the uri
  for(unsigned int i=0;i<resource_count;i++){
    unsigned int j=0;
    boolean match=true;
    while(match && j<EEPROM_RESOURCE_ALLOC_SIZE){
      //if the fields do not match
      //go to the next resource
      if(resources[i].uri[j]!=uri[j])
        match=false;
      j++;
    }
    if (match) return true;
  }
  return false;
}

boolean ResourceManager::pinInUse(uint8_t pin){
  for(unsigned int i=0;i<resource_count;i++){
    if(resources[i].pin==pin) return true;
  }
  return false;
}
