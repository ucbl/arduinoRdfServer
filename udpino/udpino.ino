/*
  UDPSendReceive.pde:
 This sketch receives UDP message strings, prints them to the serial port
 and sends an "acknowledge" string back to the sender

 A Processing sketch is included at the end of file that can be used to send
 and received messages for testing with a computer.

 created 21 Aug 2010
 by Michael Margolis

 This code is in the public domain.
 */


#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet2.h>
#include <EthernetUdp2.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008

// *** type ***
#define CON B00000000
#define NON B00010000
#define ACK B00100000
// *** code ***
#define EMPTY B00000000
#define VALID B01000011
#define CREATED B01000001 //only for POST-PUT
#define DELETED B01000010 //only for POST-PUT
#define CHANGED B01000100
#define CONTENT B01000101 //only for GET
#define PAYLOAD_START B11111111
#define M B00001000
#define PAYLOAD_MAX_SIZE 16

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0x90, 0xA2, 0xDA, 0x10, 0x18, 0x6F
};
IPAddress ip(10, 0, 0, 2);

unsigned int localPort = 5683;      // local port to listen on

// *** #define UDP_TX_PACKET_MAX_SIZE 32 ***
// buffers for receiving and sending data
char buffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet
int packetCursor=4;
int payloadCursor=0;
// URI-Path index and length
int uri[2]={0,0};
int optDelta=0;
// Request
int method=0;
int payloadStartIndex=0;
String request="";
String payload="my little pony 1 my little pony 2 my little pony 3 my little pony 4";
// Blocks
int blockIndex=0;
// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void resetVariables(){
  payloadStartIndex=uri[0]=uri[1]=blockIndex=optDelta=0;
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
  // *** check and read options ***
  if((buffer[4]!=PAYLOAD_START) && (buffer[4]!=EMPTY)){
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

void writeHeader(int type, int block, int code, int blockNum){
  writeType(type);
  writeCode(code);
  packetCursor = 4;
  
  // copy the block option into the response
  if (block>0){
    packetCursor=7;
    // 13 | length 1
    buffer[4]=B11010001;
    // delta extended 23 or 27
    if (block==1) {
      buffer[5]=B00001110;
      buffer[6]=B00000000|(blockNum<<4);
    }
    if (block==2) {
      buffer[5]=B00001010;
      // check if payload needs
      // more blocks
      int more=1;
      if(blockNum>=payload.length()/16) more=0;
      buffer[6]=B00000000|(more*M)|(blockNum<<4);
    }
  }
}

void writePayload(){
  buffer[packetCursor]=PAYLOAD_START;
  packetCursor++;
  //Serial.print("plcursor: ");
  //Serial.println(payloadCursor);
  for (int i=0;i<PAYLOAD_MAX_SIZE;i++){
    if((packetCursor<UDP_TX_PACKET_MAX_SIZE && payloadCursor<payload.length())){
      buffer[packetCursor]=payload[payloadCursor];
      packetCursor++;
      payloadCursor++;
    }
  }
  if(payloadCursor>=payload.length()) payloadCursor=0;
}

int analyzeBlock(){
  int blockValue=0;
  if((buffer[blockIndex]&1)!=0){
    if(((255&buffer[blockIndex])>>4)==13) blockValue = buffer[blockIndex+2];
    else blockValue = buffer[blockIndex+1];  
  }
  return blockValue;
}

void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.begin(9600);
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    parseHeader();
    int blockValue=analyzeBlock();
    //Serial.print("blockValue: ");
    //Serial.println(blockValue,BIN);
    if(((255&blockValue)>>4)==0) request="";
    // *** read payload if any ***
    readPayload();
    // *** treat POST with block1 ***
    // payload in the request
    if(method==2) {
      writeHeader(NON,1, CHANGED,(255&blockValue)>>4);
      //if no more blocks
      if((blockValue&M)==0){
        Serial.println(request);
      }
    }
    // *** treat GET with block2 ***
    // no payload in the request!
    if(method==1) {
      //Serial.print("pl length: ");
      //Serial.println(payload.length());
      writeHeader(NON,2, VALID,(255&blockValue)>>4);
      writePayload();
    }
    //Serial.print("bytes sent: ");
    //Serial.println(packetCursor);
    sendBuffer(packetCursor);
    // *** reset global variables ***    
    resetVariables();
    delay(10);
  }
}
