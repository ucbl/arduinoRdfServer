# ArduinoRdfServer
Web server for aduino Uno that sends its sensor data in RDF. Users are encouraged to describe their own REST services and link them to typical Arduino functions.

Once powered up, one can configure which pins go to which function, as well as enabling and disabling available services.

#Overview
The [sketch](https://github.com/ucbl/arduinoRdfServer/tree/master/WebServer_Hackfest) turns an Arduino UNO (with attached Ethernet Shield 2) into a configurable Web Server meant to communicate data retrieved from devices.

Hardcoded services described in [Hydra](http://www.hydra-cg.com/) can be enabled, disabled, and configured at run time. Any combination of known devices can be set up on the Arduino without needing to flash a new sketch.

Each service contains an input/output data type. In our example we used simplified versions of UCBL's [Prototype WoT](https://github.com/ucbl/prototype-wot) ontologies to describe e.g.Temperature and Light. Services also refer each to a function. Functions treat the input of a request and deliver data to complement the output type mentioned previously. 

> Both the functions and the input/output types are meant to be modified and do not influence the algorithm.

Packets are transmitted using a partial implementation of [CoAP](tools.ietf.org/html/rfc7252).

**Optionally**, one can run our included translate proxy to communicate in HTTP.

##Requirements
* Ethernet Shield 2
* [Arduino.ORG SDK](http://www.arduino.org/downloads) **Not compatible with the _Arduino.CC_ version**
* [ArduinoJSON](https://github.com/bblanchon/ArduinoJson) download and add to your Arduino _libraries_ folder
* [Twisted Framework](http://twistedmatrix.com/trac/) (optional)
* [TxThings](https://github.com/mwasilak/txThings) (optional)

##Installation
Download [ArduinoJSON](https://github.com/bblanchon/ArduinoJson) and add it to your Arduino _libraries_ folder

To use the proxy, make sure you have [Python2](https://www.python.org/) and [Pip](https://pypi.python.org/pypi/pip) then run:
```
pip install twisted
pip install txthings
```
Once everything is in place, make sure the proxy and the Arduino are on the same network (10.0.0.0 by default)
> Check _WebServer_Hackfest/WebServer_Hackfest.ino_ and proxy.py

Build and Flash.

##Usage

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

