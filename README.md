# ArduinoRdfServer
Web server for aduino Uno that sends its sensor data in RDF. Users are encouraged to describe their own REST services and link them to typical Arduino functions.

Once powered up, one can configure which pins go to which function, as well as enabling and disabling available services.

#Overview
The [sketch](https://github.com/ucbl/arduinoRdfServer/tree/master/WebServer_Hackfest) turns an Arduino UNO (with attached Ethernet Shield 2) into a configurable Web Server meant to communicate data retrieved from devices.

Hardcoded services described in [Hydra](http://www.hydra-cg.com/) can be enabled, disabled, and configured at run time. Any combination of known devices can be set up on the Arduino without needing to flash a new sketch.

Each service contains an input/output data type, also called ``Resource``. In our example we used simplified versions of UCBL's [Prototype WoT](https://github.com/ucbl/prototype-wot) ontologies to describe e.g.Temperature and Light. Services also refer each to a function. Functions treat the input of a request and deliver data to complement the output type mentioned previously. 

> Both the functions and the input/output types are meant to be modified and do not influence the algorithm.

Packets are transmitted using a partial implementation of [CoAP](tools.ietf.org/html/rfc7252).

**Optionally**, one can run our included translate proxy to communicate in HTTP.

## Requirements
* Ethernet Shield 2
* [Arduino.ORG SDK](http://www.arduino.org/downloads) **Not compatible with the _Arduino.CC_ version**
* [ArduinoJSON](https://github.com/bblanchon/ArduinoJson) download and add to your Arduino _libraries_ folder
* [Twisted Framework](http://twistedmatrix.com/trac/) (optional)
* [TxThings](https://github.com/mwasilak/txThings) (optional)

## Installation
Download [ArduinoJSON](https://github.com/bblanchon/ArduinoJson) and add it to your Arduino _libraries_ folder

To use the proxy, make sure you have [Python2](https://www.python.org/) and [Pip](https://pypi.python.org/pypi/pip) then run:
```
pip install twisted
pip install txthings
```
Once everything is in place, make sure the proxy and the Arduino are on the same network (10.0.0.0 by default)
> Check _WebServer_Hackfest/WebServer_Hackfest.ino_ and proxy.py

Build and Flash.

## Usage

This version does not implement DHCP. The default IP is 10.0.0.2.
> To modify the default IP edit the line containing: IPAddress ip(10, 0, 0, 2);

Once connected and powered up, requesting a GET on root ``/`` will display the services available and their description. In order to use a service you must first enable it.

The ``enable`` request is a PUT, and takes in a JSON document containing the following:
* "uri" : the service identifier e.g. tempSense ``string``
* "pin_count" : the amount of pins on the Arduino that are going to be used ``int``
* "pins" : an array of pins mentioned above ``int[]``

It enables the service on the given pins

The ``reset`` request is an empty PUT. It resets all services.

> Enabled services stay enabled as long as ``reset`` is not requested, even if power is down or a new sketch is flashed
> This is done thanks to the fact configuration is stored in [EEPROM](https://www.arduino.cc/en/Reference/EEPROM)

## Extensions

### Adding a Service

To add a service (written in Hydra), simply insert it into ``const char CAPABILITIES[] PROGMEM`` in Semantic.h.
It must contain the following fields:
* "@id" : Which defines the URL
* "@type" : Compulsory "hydra:Operation" : used by the algorithm to register a service.
* "method" : GET, POST, PUT (implemented so far)
* "label" : Optional, usefull for debug
* "expects" : Resource identifier for the input data (POST, PUT)
* "returns" : Resource identifier for the output (GET, POST)

The service still needs to call a function (and return a Resource).

#### Adding a Function triggered by a service

To add a function declare it as: ``void function_name(uint8_t pin_count, uint8_t pins[n], JsonObject& root, uint8_t results[6])`` in Semantic.h and Semantic.cpp.
* uint8_t pin_count : number of pins used by the service (same as described when enabling the service, see [Usage](https://github.com/ucbl/arduinoRdfServer/edit/master/README.md#Usage)
* uint8_t pins[n] : n being equal to ``pin_count``
* JsonObject& root : [ArduinoJSON](https://github.com/bblanchon/ArduinoJson) representation of the parsed request **expected** JSON document.
* uint8_t results : array used to store data that will be used in the response.

Finally in WebServer_Hackfest.ino, you must link the function to the corresponding service by calling ``coap.addOperationFunction(&FUNCTION_NAME, (char* )"SERVICE_ID");`` at the beginning of the setup REGARDLESS the service will be enabled or not.

The ``uint8_t results`` array is used when composing the response. It is used to complement the output JSON document that represents the requested Resource. More information below.

#### Adding an Resource

To add a new Resource, create a variable ``const char VARIABLE_NAME[] PROGMEM`` in Semantic.h containing a JSON-LD document.
Make sure to include a ``"@type"`` field: it will be used to identify the variable when reading the service's ``"expects"`` or ``"returns"`` fields.

When a Resource is written into the socket, the algorithm checks for double empty strings, i.e. "". These are the spots where the ``uint8_t results`` array values mentioned in the previous paragraph will be inserted. For example the ``const char TEMPERATURE[] PROGMEM`` Resource has the ``"value"`` field set to **""**, so when writing a temperature into the response, the ``uint8_t results`` contains the result of a temperature read.

> For now only ONE result value can be stored.
> As we write bytes into the socket interpreted in ASCII by the receiver, in order to write 33.5 results = [051,051,046,053,NULL,NULL]

Finally in Semantic.h add your new variable to the list ``PGM_P const json_resources[] PROGMEM`` and edit ``#define RESOURCE_COUNT`` to match the number of entries in the list.





