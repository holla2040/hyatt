#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h> // curl -F "image=@/tmp/arduino_build/ESP8266Template.ino.bin" myLoc.local/upload
#include <WebSocketsServer.h>
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager
#include <EEPROM.h>
#include <ArduinoJson.h>

/*
  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in `ls -A1`; do echo $file;curl -F "file=@$PWD/$file" myLoc.local/edit; done
*/

// #define VERBOSE

// comment to disable TCP Socket Server,  socat TCP:hyatt.local:23 -,raw,echo=0
#define TCPSERVER
#define MAX_SRV_CLIENTS 2
#define TCPSERVERPORT 23

#define NAMELEN 20
#define LABELLEN 50
#define LED 2

char name[NAMELEN];
char label[LABELLEN];
uint32_t heartbeatTimeout;
uint32_t heartbeat; // this is heartbeat interval in mS

ESP8266WebServer httpServer(80);
WebSocketsServer webSocketServer = WebSocketsServer(81);

#ifdef TCPSERVER
WiFiServer tcpServer(TCPSERVERPORT);
WiFiClient tcpServerClients[MAX_SRV_CLIENTS];
#endif

ESP8266HTTPUpdateServer httpUpdater;
const char *update_path = "/upload";
WiFiManager wifiManager;
#include "fs.h"

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
#ifdef VERBOSE  
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
#endif
  switch(type) {
    case WStype_DISCONNECTED:
#ifdef VERBOSE  
      Serial.printf("[%u] Disconnected!\r\n", num);
#endif
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocketServer.remoteIP(num);
#ifdef VERBOSE  
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
#endif
      }
      break;
    case WStype_TEXT:
    Serial.print((const char *)payload);
    webSocketServer.broadcastTXT("C");
#ifdef VERBOSE  
      Serial.printf("[%u] get Text: %s\r\n", num, payload);
#endif
/*
      if (strcmp(LEDON, (const char *)payload) == 0) {
        writeLED(true);
      }
      else if (strcmp(LEDOFF, (const char *)payload) == 0) {
        writeLED(false);
      }
      else {
#ifdef VERBOSE  
        Serial.println("Unknown command");
#endif
      }
*/

      // send data to all connected clients
      webSocketServer.broadcastTXT(payload, length);
      break;
    case WStype_BIN:
#ifdef VERBOSE  
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
#endif
      hexdump(payload, length);

      // echo data back to browser
      webSocketServer.sendBIN(num, payload, length);
      break;
    default:
#ifdef VERBOSE  
      Serial.printf("Invalid WStype [%d]\r\n", type);
#endif
      break;
  }
}


//get heap status, analog input value and all GPIO statuses in one json call
void handleStatus() {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    httpServer.send(200, "text/json", json);
    json = String();
}

void configLoad() {
  StaticJsonDocument<1024> doc;
  File file = SPIFFS.open("/config.json", "r");
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
#ifdef VERBOSE  
    Serial.println(F("Failed to read file, using default configuration"));
#endif
  } else {
    JsonObject config = doc.as<JsonObject>();
    strlcpy(name,config["name"] | "myName", NAMELEN);
#ifdef VERBOSE  
    Serial.print("name:      ");
    Serial.println(name);
#endif

    strlcpy(label,config["label"] | "myLabel", LABELLEN);
#ifdef VERBOSE  
    Serial.print("label:     ");
    Serial.println(label);
#endif

    heartbeat = config["heartbeat"] | 1001;
#ifdef VERBOSE  
    Serial.print("heartbeat: ");
    Serial.println(heartbeat);
#endif

    file.close();
  }
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += httpServer.uri();
  message += "\nMethod: ";
  message += (httpServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpServer.args();
  message += "\n";

  for (uint8_t i = 0; i < httpServer.args(); i++) {
    message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";
  }

  httpServer.send(404, "text/plain", message);
}

void setup(void) {
  char c;
  int i;
  Serial.begin(115200);
#ifdef VERBOSE  
  Serial.println("\n hyatt begin");
#endif
  pinMode(LED, OUTPUT);

  fsSetup();
#ifdef VERBOSE  
  Serial.println("filesystem started");
#endif

  /* default values, updated by config.json parsing */
  heartbeat = 1001;
  strcpy(name,"hyatt");

  configLoad();

  // wifiManager.resetSettings();
  wifiManager.setDebugOutput(0);
  wifiManager.autoConnect(name);

  httpServer.on("/status", HTTP_GET, handleStatus );
  httpServer.on("/reset", []() {
    httpServer.send(200, "text/plain", "reseting wifi settings\n");
    wifiManager.resetSettings();
  });
  httpServer.on("/reboot", []() {
    httpServer.send(200, "text/plain", "rebooting\n");
    delay(1000);
    ESP.reset();
  });
  httpServer.on("/reload", []() {
    httpServer.send(200, "text/plain", "reloading\n");
    configLoad();
  });
  httpServer.on("/heartbeat", []() {
    httpServer.send(200, "text/plain", "heartbeat set\n");
    heartbeat = (httpServer.arg("value")).toInt();
  });


  httpUpdater.setup(&httpServer, update_path);
  httpServer.begin();
#ifdef VERBOSE  
  // Serial.print("IP   ");
  // Serial.println(WiFi.localIP().toString());
  Serial.println("HTTP server started");
  Serial.println("HTTP updater started");
#endif

  if (MDNS.begin(name)) {
#ifdef VERBOSE  
      Serial.print ("MDNS responder started http://");
      Serial.print(name);
      Serial.println(".local");
#endif
  }

  webSocketServer.begin();
  webSocketServer.onEvent(webSocketEvent);
#ifdef VERBOSE  
  Serial.println("WebSocketServer started");
#endif

#ifdef TCPSERVER
  tcpServer.setNoDelay(true);
  tcpServer.begin();
#ifdef VERBOSE  
  Serial.print("tcpServer started on port ");
  Serial.println(TCPSERVERPORT);
#endif
#endif

}

#ifdef TCPSERVER
void tcpServerLoop() {
  int i;
  char c;
  if (tcpServer.hasClient()) {
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      //find free/disconnected spot
      if (!tcpServerClients[i] || !tcpServerClients[i].connected()) {
        if (tcpServerClients[i]) tcpServerClients[i].stop();
        tcpServerClients[i] = tcpServer.available();
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient tcpServerClient = tcpServer.available();
    tcpServerClient.stop();
  }

  for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (tcpServerClients[i] && tcpServerClients[i].connected()) {
      while (tcpServerClients[i].available()) {
        Serial.print((char)tcpServerClients[i].read());
      }
    }
  }

  if (Serial.available()) {
    c = Serial.read();
    for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (tcpServerClients[i] && tcpServerClients[i].connected()) {
        tcpServerClients[i].write(c);
      }
    }
  }
}

void tcpServerWrite(char *buf, uint16_t len) {
  for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (tcpServerClients[i] && tcpServerClients[i].connected()) {
      tcpServerClients[i].write(buf,len);
    }
  }
}

#endif
  

void loop(void) {
  char line[40];
  httpServer.handleClient();
  webSocketServer.loop();
  tcpServerLoop();

  if (millis() > heartbeatTimeout) {
    digitalWrite(LED,!digitalRead(LED));

    sprintf(line,"uptime:%d",millis()/25);
    webSocketServer.broadcastTXT(line,strlen(line));

    sprintf(line,"led:%d",!digitalRead(LED));
    webSocketServer.broadcastTXT(line,strlen(line));

    sprintf(line,"name:%s",name);
    webSocketServer.broadcastTXT(line,strlen(line));

    sprintf(line,"label:%s",label);
    webSocketServer.broadcastTXT(line,strlen(line));

    sprintf(line,"uptime:%d",millis()/25);
    webSocketServer.broadcastTXT(line,strlen(line));

    heartbeatTimeout = millis() + heartbeat;
  }

  ESP.wdtFeed(); 

  yield();
}
