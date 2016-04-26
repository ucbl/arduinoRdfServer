#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet2.h>
#include <EthernetUdp2.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <avr/pgmspace.h>
#include <EEPROM.h>
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

//////////////////////////////////////////////////////////////
// CAPABILITIES
//////////////////////////////////////////////////////////////

const char CAPABILITIES[] PROGMEM = 
  //"{"
    "{"
      "\"@id\": \"vocab:CapabilityTemperatureSense\","
      "\"@type\": [\"hydra:Resource\", \"vocab:Capability\"],"
      "\"subClassOf\": null,"
      "\"label\": \"CapabilityTemperatureSense\","
      "\"description\": \"Capability that queries a temperature sensor\","
      "\"supportedOperation\": ["
        "{"
          "\"@id\": \"_:temperatureSense\","
          "\"@type\": \"hydra:Operation\","
          "\"method\": \"GET\","
          "\"label\": \"temperatureSense\","
          "\"description\": \"Retrieves a temperature measured by the temperature sensor\","
          "\"expects\": null,"
          "\"returns\": \"vocab:Temperature\","
          "\"statusCodes\": []"
        "}"
      "]"
    "}\0";
  //"}\0";
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:

byte mac[] = {
  0x90, 0xA2, 0xDA, 0x10, 0x18, 0x6F
};
IPAddress ip(10, 0, 0, 2);

unsigned int localPort = 5683;      // local port to listen o

//////////////////////////////////////////////////////////////
// COAP
//////////////////////////////////////////////////////////////
// *** #define UDP_TX_PACKET_MAX_SIZE 80 ***
// buffers for receiving and sending data
char buffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet
int packetCursor=4;
int payloadCursor=0;
// uri and payload correspondance
typedef struct{
  String path;
  const char* const_payload;
} uripayload_t;

uripayload_t uripayload[5];
// URI-Path index and length
int tokenLength=0;
int uri[2]={0,0};
String path="/";
int optDelta=0;
// Request
int method=0;
int payloadStartIndex=0;
String request="";
int const_payload_index=-1;
int payloadLen = 0;
// Blocks
int blockIndex=0;
// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void resetVariables(){
  payloadStartIndex=uri[0]=uri[1]=blockIndex=optDelta=tokenLength=0;

  memset(buffer,0,UDP_TX_PACKET_MAX_SIZE);
  packetCursor=4;
}

void sendBuffer(int sendSize){
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(buffer,sendSize);
  Udp.endPacket();
}

void writeCode(char code){
  buffer[1]&=B00000000;
  buffer[1]|=code;
}
void writeType(char type){
  buffer[0]&=B11001111;
  buffer[0]|=type;
}

// *** return TRUE if there are more options following ***
boolean readOptionDesc(){
  int optNumber=((255&buffer[packetCursor])>>4)+optDelta;
  if (((255&buffer[packetCursor])>>4)==13) optNumber+=int(255&buffer[packetCursor+1]);
  int optLength=int(buffer[packetCursor]&B00001111);
  // 11: URI-Path
  if(int(optNumber)==11){
    uri[0]=packetCursor+1;
    uri[1]=optLength;
  }
  // *** if BLOCK ***
  if(optNumber==23 || optNumber==27) {
    blockIndex=packetCursor;
  }
  
  // 13:  An 8-bit unsigned integer follows the initial byte and
  // indicates the Option Delta minus 13.
  if(((255&buffer[packetCursor])>>4)==13){
    packetCursor++;
  }

  packetCursor+=int(optLength)+1;
  optDelta=optNumber;
  if(((255&buffer[packetCursor])!=PAYLOAD_START) && ((255&buffer[packetCursor])!=EMPTY)) return true;
  else return false;
}

void readPayload(){
  if (payloadStartIndex!=0)
    for(int i=payloadStartIndex;i<sizeof(buffer);i++) request+=buffer[i];
}

void parseHeader(){
  // *** store packet into buffer ***
  Udp.read(buffer, UDP_TX_PACKET_MAX_SIZE);
  tokenLength=(15&buffer[0]);
  packetCursor+=tokenLength;
  // *** check and read options ***
  if((buffer[packetCursor]!=PAYLOAD_START) && (buffer[packetCursor]!=EMPTY)){
    boolean options = true;
    while(options){
      options = readOptionDesc();
    }
  }
  // *** check if payload ***
  if((255&buffer[packetCursor])==PAYLOAD_START){
    payloadStartIndex=packetCursor+1;
  }
  method = int(buffer[1]);
}

void writeHeader(int type, int block, int code, int blockNum, int payloadLength){
  writeType(type);
  writeCode(code);
  packetCursor = 4+tokenLength;
  
  if (block>0){
    // 13 | length 1
    buffer[packetCursor]=B11010001;
    // delta extended 23 or 27
    if (block==1) {
      buffer[packetCursor+1]=B00001110;
      buffer[packetCursor+2]=B00000000|(blockNum<<4);
    }
    if (block==2) {
      buffer[packetCursor+1]=B00001010;
      // check if payload needs
      // more blocks
      int more=1;
      if(blockNum>=payloadLength/PAYLOAD_MAX_SIZE) more=0;
      buffer[packetCursor+2]=B00000010|(more*M)|(blockNum<<4);
    }
    packetCursor+=3;
  }
}

int getPayloadLength(const char* payloadIndex){
  int len =0;
  while(pgm_read_byte(payloadIndex)!='\0'){
    len++;
    payloadIndex++;
  }
  return len;
}

void writePayload(const char* payloadStartIndex, int payloadLength){
  buffer[packetCursor]=PAYLOAD_START;
  packetCursor++;
  //Serial.print("plcursor: ");
  //Serial.println(payloadCursor);
  for (int i=0;i<PAYLOAD_MAX_SIZE;i++){
    if((packetCursor<UDP_TX_PACKET_MAX_SIZE && payloadCursor<payloadLength)){
      buffer[packetCursor]=pgm_read_byte(payloadStartIndex+payloadCursor);
      packetCursor++;
      payloadCursor++;
    }
  }
  //once the last block is written
  if(payloadCursor>=payloadLength) {
    payloadCursor=payloadLen=0;
    const_payload_index = -1;
  } 
}

int analyzeBlock(){
  int blockValue=0;
  if((buffer[blockIndex]&1)!=0){
    if(((255&buffer[blockIndex])>>4)==13) blockValue = buffer[blockIndex+2];
    else blockValue = buffer[blockIndex+1];  
  }
  return blockValue;
}

//////////////////////////////////////////////////////////////
// APP LEVEL
//////////////////////////////////////////////////////////////

int getUriPayload(){
  //uri[0] = uri start index in buffer[]
  // uri[1] = uri length
  for (int i=0; i<uri[1]; i++) 
    path+=buffer[uri[0]+i];
  for (int i=0; i<10; i++)
    if (path.equals(uripayload[i].path))
      return i;
  return -1;
}

void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.begin(9600);
  uripayload[0].path="/";
  uripayload[0].const_payload=CAPABILITIES;
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    parseHeader();
    int blockValue=analyzeBlock();
    // if it's the first block
    if(((255&blockValue)>>4)==0){
      // *** GET ***
      // get the payload index in progmem
      // for current request
      const_payload_index = getUriPayload();
      payloadLen = getPayloadLength(uripayload[const_payload_index].const_payload);
      // *** POST ***
      // incoming request payload
      request="";
    }
    // *** read payload if any ***
    readPayload();
    // *** treat GET with block2 ***
    // no payload in the request!    
    if(method==1) {
      // get the payload for the current uri
      writeHeader(ACK,2, VALID,(255&blockValue)>>4, payloadLen);
      writePayload(uripayload[const_payload_index].const_payload, payloadLen);  
    
    }
    // *** treat POST with block1 ***
    // payload in the request
    if(method==2) {  
      writeHeader(ACK,1, CHANGED,(255&blockValue)>>4, 0);
      if((blockValue&M)==0){
        // Do something with the received payload
        Serial.println(request);
      }
    }
    //Serial.print("bytes sent: ");
    //Serial.println(packetCursor);
    sendBuffer(packetCursor);
    // *** reset global variables ***    
    resetVariables();
    delay(10);
  }
}
