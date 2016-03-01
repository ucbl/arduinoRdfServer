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

//Request Code
int request = 0;

// *** MODEL ***
void switchLight(boolean on){
  if(on) digitalWrite(LIGHT_CODE, HIGH);
  else digitalWrite(LIGHT_CODE, LOW);
}
float getTemp(int analog) {
  //Formula found at https://learn.adafruit.com/tmp36-temperature-sensor/using-a-temp-sensor
  return ((analogRead(analog) * 3300. / 1024.) - 500.) / 10.;
}

void buildResponseHeader(int HTTP){
  if (HTTP==200){
    header =  
      "HTTP/1.1 200 OK\n"
      "Content-Type: text/html\n"
      "Access-Control-Allow-Origin: *\n"
      "Access-Control-Allow-Methods: POST, GET, PUT, OPTIONS\n"
      "Connection: close";
  }
}

void resetVariables(){
  method = location = device = value = query = url = header = "";
  firstLine = true;
  request=0;
}

void parseRequest() {
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
    request|=1<<5;
    if(String(value)!=""){//set value
      request|=1;
    }
  } else if(String(method)=="PUT") {
    request|=2<<5;
    if(String(value)!=""){//set value
      request|=1;
    }
  } else if (String(method)=="OPTIONS"){
    request|=3<<5;
  } else if (String(method)!="GET"){//GET is default
    //method error
    request|=1<<7;
    request|=1<<5; 
  }

  // *** move device 1 bit ***
  if(String(device)=="light"){
    request |=LIGHT_CODE<<1;
  }
  else if(String(device)=="temperature"){  
    request |=TEMPERATURE_CODE<<1;
  } else { 
    //url error
    request|=1<<7;
    request|=2<<5; 
  }
}
/*
  Perform an operation using the request
  EG.
  00100111 -> POST, light 0011, value
  */
void performOperation(){
  if((request>>7)!=1){// no error
    if(((request>>5)&3)==1) { //POST
      if((((request>>1)&15)==LIGHT_CODE) && ((request&1)==1)) {
        if(String(value)=="on") switchLight(true);
        if(String(value)=="off") switchLight(false);
      }
    } else if (((request>>5)&3)==0) {//GET
      if((((request>>1)&15)==TEMPERATURE_CODE) && ((request&1)==0)) {
        value=String(getTemp(A2));
      }
    }
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
          parseRequest();
          performOperation();
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
          }*/
          client.print("<!DOCTYPE HTML>\n");
          client.print("<html>\n");
          client.print("<body>\n");
          client.print("<script type=\"application/ld+json\">\n");
          client.print("{\n");
          client.print("\"@context\":\"applicationContext.jsonld\"");
          client.print(",\n\"query\": \""+query+"\"");
          client.print(",\n\"request\": "+String(request));
          if(method!="") client.print(",\n\"method\": \""+method+"\"");
          if(location!="")  client.print(",\n\"location\": \""+location+"\"");
          if(device!="")  client.print(",\n\"device\": \""+device+"\"");
          if(value!="")  client.print(",\n\"value\": \""+value+"\"");
          client.println("\n}");
          client.print("</script>\n");
          client.print("</body>\n");
          client.println("</html>");

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


