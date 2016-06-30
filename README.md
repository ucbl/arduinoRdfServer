# ArduinoRdfServer
Web server for aduino Uno that sends its sensor data in RDF. Users are encouraged to describe their own REST services and link them to typical Arduino functions.

Once powered up, one can configure which pins go to which function, as well as enabling and disabling available services.

##Requirements
* Ethernet Shield 2
* [Arduino.ORG SDK](http://www.arduino.org/downloads) **Not compatible with the _Arduino.CC_ version**
* [ArduinoJSON](https://github.com/bblanchon/ArduinoJson) download and add to your Arduino _libraries_ folder
* [Twisted Framework](http://twistedmatrix.com/trac/) (optional)
* [TxThings](https://github.com/mwasilak/txThings) (optional)

##Installation
Download [ArduinoJSON](https://github.com/bblanchon/ArduinoJson) and add it to your Arduino _libraries_ folder

Packets are transmitted with [CoAP](tools.ietf.org/html/rfc7252).
Optionally, one can run our included translate proxy to communicate in HTTP.
To do so, make sure you have [Python2](https://www.python.org/) and [Pip](https://pypi.python.org/pypi/pip) then run:
```
pip install twisted
pip install txthings
```
Once everything is in place, make sure the proxy and the Arduino are on the same network (10.0.0.0 by default)
> Check _WebServer_Hackfest/WebServer_Hackfest.ino_ and proxy.py

Build and Flash.


