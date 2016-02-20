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
char query[]= "";
int queryLength = 0;
boolean firstLine = true;
//Request parameters
//String requestParameters = "";

//Request parts
char method[] = "";
char location[] = "";
char device[] = "";
char value[] = "";

// *** MODEL ***
float getTemp(int analog) {
  //Formula found at https://learn.adafruit.com/tmp36-temperature-sensor/using-a-temp-sensor
  return ((analogRead(analog) * 3300. / 1024.) - 500.) / 10.;
}

byte parseRequest() {
  int start = 0; //start index of the current part
  int split = 0; //0 for method, 1 for URL and 2 after (protocol)
  for(int i=0; query[i] != '\0' && split < 2; i++) {
      Serial.println();
      Serial.println("test");

    if(query[i] == ' ' || split == 0) {
      method[i] = '\0';
      split++;
      start = i;
      continue;
    }
    //Processing the interesting parts of the query
    if(split == 0) { //Method
      method[i- start] = query[i];
    } else { //URL
      int split2 = 0; //0 for location, 1 for device type, 2 for value
      if(query[i] == ' ' || query[i] == '/') {
        if(split2 == 0) {
          location[i] = '\0';
        } else if(split2 == 1){
          device[i - start] = '\0';
        } else {
          value[i - start] = '\0';
        }
        split2++;
        start = i;
        continue;
      }
      if(split2 == 0) { //location
        location[i- start] = query[i];
      } else if(split2 == 1){
        device[i- start] = query[i];
      } else {
        value[i- start] = query[i];
      }
    }
    Serial.println("** parseRequest ** ");
    Serial.print("Method: ");
    Serial.println(method);
    Serial.print("Location: ");
    Serial.println(location);
//    Serial.println();
//    Serial.println();
  }
  /*Construct the response bit by bit (from highest to lowest level):
      Error: 0 = no error, 1 = error
      If error :
        0000000 = parse error, 0100000 = method error, 1000000 = url error, 11XXXXX = other errors
      Else :
        Method: 00 = GET, 01 = POST, 10 = PUT, 11 = OPTIONS
        Sensor / actuator type: 4 bits, according to the configuration
        Value (only for POST & PUT): 0 = value not set, 1 = value set
  */
  byte result=0;
  if(method == "GET") {
    result = 0;
    
    //For the moment, assume we only want temperature
  }
  return result;
}



// *** SETUP ***
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
/*  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
*/

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
//    Serial.flush();
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        //Store the first line of the query
        if(firstLine) {
          if(c != '\n') {
            query[queryLength++] = c;
          } else {
            query[queryLength] = '\0';
            firstLine = false;
          }
        }
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          //parse the request
          byte request = parseRequest();
          // send the http response header
          //*******sendOkHeaders(client);
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json"); //Response will be JSON-LD
          //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println("Access-Control-Allow-Origin: *");  //CORS header, as the server is expected to be queried in cross-domain
          client.println("Access-Control-Allow-Methods: POST, GET, PUT, OPTIONS");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          //******************************//
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
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
          client.print("Request int value: ");
          client.println(request);
          client.print("Method: ");
          client.println(method);
          client.print("location: ");
          client.println(location);
          client.print("device: ");
          client.println(device);
          client.print("Value: ");
          client.println(value);
          client.println("</html>");

          break;
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
    // close the connection:
    client.stop();
    Serial.println("client disconnected");

    //Reinit variables for next request:
    queryLength = 0;
    firstLine = true;
    //requestParameters = "";
    method[0] = location[0] = device[0] = value[0] = '\0';
  }
}


