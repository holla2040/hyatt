# hyatt wifiBridge

Based on holla2040/ESP8266Template
https://github.com/holla2040/ESP8266Template

Supports
* ESP8266WebServer
* ESP8266HTTPUpdateServer
* WiFiManager
* MDNS
* WebSockets
* Non-delay led blink with heartbeat interval config.json support
* SPIFFS
* ArduinoJson
* config.json parsing
* index.htm has ajax and websocket example code
* option compile of TCP Socket - Serial Port bridging

Sample URLs (also look at the curl make targets)
* status query,http://hyatt.local/status
* reset wifi manager, http://hyatt.local/reset
* reboot, http://hyatt.local/reboot
* changing runtime hearbeat interval, http://hyatt.local/heartbeat?value=50

