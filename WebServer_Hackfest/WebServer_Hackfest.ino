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

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0x90, 0xA2, 0xDA, 0x10, 0x18, 0x6F
};
IPAddress ip(192, 168, 1, 2);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

// *** RESOURCE CONFIG ***
#define TEMPERATURE_CODE 0
#define BLUETOOTH_CODE 1
//... (max: 4 bytes -> 16 types of devices)

// *** Request handling ***
String query= "";
int queryLength = 0;
boolean firstLine = true;
//Request parameters
//String requestParameters = "";

//Request parts
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
  int start = 0;
  int i=0;
  int result=-1;
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
    /*query+='\0';
    location+='\0';
    device+='\0';
    value+='\0';*/
  }
   
  /*
  Construct the response bit by bit (from highest to lowest level):
      Error: 0 = no error, 1 = error
      If error :
        0000000 = parse error, 0100000 = method error, 1000000 = url error, 11XXXXX = other errors
      Else :
        Method: 00 = GET, 01 = POST, 10 = PUT, 11 = OPTIONS
        Sensor / actuator type: 4 bits, according to the configuration
        Value (only for POST & PUT): 0 = value not set, 1 = value set
  */
  if(String(method)=="GET") {
    result = 0;
    //For the moment, assume we only want temperature
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
          if(c != '\n') {
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
          //client.println("<!DOCTYPE HTML>");
          //client.println("<html>");
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
          client.print("Query: ");
          client.println(query);
          client.print("Request int value (-1=error): ");
          client.println(request);
          client.print("Method: ");
          client.println(method);
          client.print("location: ");
          client.println(location);
          client.print("device: ");
          client.println(device);
          client.print("Value: ");
          client.println(value);
          //client.println("</html>");
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


