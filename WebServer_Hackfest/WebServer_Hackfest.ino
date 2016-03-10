/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 ArduinoJson library by Benoît Blanchon
 https://github.com/bblanchon/ArduinoJson

 Modification: Lionel Médini, June 2015
 Modification: Remy Rojas, March 2016

*/

// *** NETWORK CONFIG ***

#include <SPI.h>
#include <Ethernet2.h>
#include <ArduinoJson.h>

byte mac[] = {
  0x90, 0xA2, 0xDA, 0x10, 0x18, 0x6F
};
IPAddress ip(192, 168, 1, 2);

EthernetServer server(80);

// *** RESOURCE CONFIG **
#define BLUETOOTH_CODE 1
#define TEMPERATURE_CODE 2
#define LIGHT_CODE 3
//... (max: 4 bits -> 16 types of devices)

// *** MODEL ***
void switchLight(boolean on){
  if(on) digitalWrite(LIGHT_CODE, HIGH);
  else digitalWrite(LIGHT_CODE, LOW);
}
float getTemp(int analog) {
  //Formula found at https://learn.adafruit.com/tmp36-temperature-sensor/using-a-temp-sensor
  return ((analogRead(analog) * 3300. / 1024.) - 500.) / 10.;
}

// *** Store first input header line ***
boolean receiveInput(EthernetClient& client, String& input){
  while(client.connected()){
    if(client.available()){
      char c = client.read();
      if(c!='\r' && c!='\n') input += c;
      else return true;
    }
  }
  return false;
}
// *** Determine the client's method ***
int parseMethod(String& input){
  int firstSpaceIndex = input.indexOf(' ');
  if(input.substring(0,firstSpaceIndex).compareTo("POST")==0) return 1;
  else if(input.substring(0,firstSpaceIndex).compareTo("PUT")==0) return 2; 
  else if(input.substring(0,firstSpaceIndex).compareTo("OPTIONS")==0) return 3;
  // *** GET by default ***
  else return 0;
}

// *** Extract a specific element from the requested Url ***
// Determine which Element to extract by
// indexing the correct prefix slash
void extractUrlElement(String& input, int element, String& output){
  // Find URL start/end indexes
  int slashIndex = input.indexOf('/');
  int urlEndIndex = input.indexOf(' ', slashIndex);
  bool error = false;
  // Move to the correct element
  for (int i=0; i<element; i++) {
    slashIndex = input.indexOf('/',slashIndex+1);
    if(slashIndex > urlEndIndex) error = true;
  }
  // Store the element, flag if invalid
  if(error)  output = "--invalid--";
  else
  // Check whether it's the last element of the URL 
  if(input.indexOf('/', slashIndex+1) > urlEndIndex){
    output = input.substring(slashIndex+1, input.indexOf(' ',slashIndex+1));
  } 
  else output = input.substring(slashIndex+1, input.indexOf('/',slashIndex+1));
} 

/*
  Perform an operation using the request
  EG.
  00100111 -> POST, light 0011, value
  */
void performOperation(String& device, String& value){
  if(device.compareTo("lights")==0){
    if(value.compareTo("on")==0) switchLight(true);
    if(value.compareTo("off")==0) switchLight(false);
  }
} 
// *** Build Header according to a status code ***
void buildResponseHeader(EthernetClient& client, int code){
  if (code=200){
    client.println("HTTP/1.1 200 OK"); 
    client.println("Content-Type: application/ld+json");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Access-Control-Allow-Methods: POST, GET, PUT, OPTIONS");
    client.println("Connection: close");
    client.println();
  }
}

// *** SETUP ***
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  pinMode(LIGHT_CODE,OUTPUT);
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

// *** NORMAL FUNCTIONING ***
void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    String input=""; 
    if (receiveInput(client,input)) {
      int method = parseMethod(input);
      String location = "";
      String device = "";
      String value = "";
      extractUrlElement(input, 0, location);
      extractUrlElement(input, 1, device);
      extractUrlElement(input, 2, value);
      
      performOperation(device, value);
      
      StaticJsonBuffer<500> jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();
      root["input"]= input;
      root["method"]= method;
      JsonObject& url = root.createNestedObject("url");
      url["location"]= location;
      url["device"]= device;
      url["value"]= value;

      buildResponseHeader(client, 200);
      root.prettyPrintTo(client);
      delay(1);
      client.stop();
      Serial.println("client disconnected");
    }
  }
}


/*Construct the response bit by bit (from highest to lowest level):
  0   1 2   3 4 5 6   7
  X | X X | X X X X | X 
  0 | method | device | value set/not set
  Method: 00 = GET, 01 = POST, 10 = PUT, 11 = OPTIONS
  Sensor / actuator type: 4 bits, according to the configuration
  Value (only for POST & PUT): 0 = value not set, 1 = value set
  
  0   1 2 3 4 5 6 7
  X | X X X X X X X
  1 | error code
  Error code: 0000000 = parse error, 0100000 = method error, 1000000 = url error, 11XXXXX = other errors
*/
