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

 Modification: Lionel Médini, June 2015
*/

// *** NETWORK CONFIG ***

#include <SPI.h>
#include <Ethernet2.h>

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

// *** Request handling ***
boolean firstLine = true;

//Request parts
String query= "";
String method = "";
String url = "";
String location = "";
String device = "";
String value = "";
String header = "";

// *** MODEL ***
float getTemp(int analog) {
  //Formula found at https://learn.adafruit.com/tmp36-temperature-sensor/using-a-temp-sensor
  return ((analogRead(analog) * 3300. / 1024.) - 500.) / 10.;
}

void buildResponseHeader(int HTTP){
  if (HTTP==200){
    header =  
      "HTTP/1.1 200 OK\n"
      "Content-Type: application/json\n"
      "Access-Control-Allow-Origin: *\n"
      "Access-Control-Allow-Methods: POST, GET, PUT, OPTIONS\n"
      "Connection: close";
  }
}

void resetVariables(){
  method = location = device = value = query = url = header = "";
  firstLine = true;
}

int parseRequest() {
  int result= 0;
  int start = 0;
  int i=0;
  //************* METHOD
  while(query[i]!=' '){
    method+=query[i];
    i++;
  }
  i++;//count the blank space
  start = i;//write from 0, not i
  //************* URL
  while(query[i]!=' '){
    url+= query[i];
    i++;
  }
  int urlLength = i-start;
  //we have a method and it's url properly formed
  //split into 0 location, 1 device, 2 value
  start=0;//start index of the current part
  int split = -1;
  //URL parse
  for(i=0;i<urlLength;i++){
    if(url[i]=='/') {
      split++;//update split
      start = i+1;//url[i]='/' so start is at i+1
    } else {
      if(split==0) location+=url[i];
      if(split==1) device+=url[i];
      if(split==2) value+=url[i];
    }
  }
   
  /*
  Construct the response bit by bit (from highest to lowest level):
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
  // *** move method 5 bits ***
  if(String(method)=="POST"){
    result|=1<<5;
  } else if(String(method)=="PUT") {
    result|=2<<5;
  } else if (String(method)=="OPTIONS"){
    result|=3<<5;
  } else if (String(method)!="GET"){//GET is default
    //method error
    result|=1<<7;
    result|=1<<5; 
  }

  // *** move device 1 bit ***
  if(String(device)=="light"){
    result |=LIGHT_CODE<<1;
  }
  else if(String(device)=="temperature"){  
    result |=TEMPERATURE_CODE<<1;
  } else { 
    //url error
    result|=1<<7;
    result|=2<<5; 
  }
  return result;
}

// *** SETUP ***
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
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
    Serial.println("new client");
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        //Store the first line of the query
        if(firstLine) {
          if(c != '\n' && c!='\r') {
            query+=c;
          } else {
            firstLine = false;
          }
        }
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // *** Parse the first line
          int request=parseRequest();
          // *** sendOkHeaders(client);
          buildResponseHeader(200);
          //header built
          client.println(header);
          client.println();
          //******************************//
          // output the value of each analog input pin
/*          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            float sensorReading = getTemp(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
*/         
          client.println("{");
          client.print("\"query\": \"");
          client.println(query+"\",");
          client.print("\"request\": ");
          client.println(String(request)+",");
          client.print("\"method\": \"");
          client.println(method+"\",");
          client.print("\"location\": \"");
          client.println(location+"\",");
          client.print("\"device\": \"");
          client.println(device+"\"");
          if(value!=""){
            client.print(",\n\"value\":");
            client.println(value);  
          }
          client.println("}");
          resetVariables();
          // close the connection:
          client.stop();
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    Serial.println("client disconnected");
  }
}


