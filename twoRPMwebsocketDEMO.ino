#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
// spiffs makes the file system 
#include "SPIFFS.h"
#include <Arduino_JSON.h>   
#include <stdio.h>

//TAKEN FROM https://randomnerdtutorials.com/esp32-websocket-server-sensor/

// RPM INSTANTIATIONS 
int hall_pin = 26;        // first RPM sensor 
int hall_pin2 = 27;       // second RPM sensor 

float total_time = 0; 
int hall_count = 0; 

const char* ssid = "Dan";
const char* password = "muanha21";

// creating an AsyncWebServer object on port 80
AsyncWebServer server(80);
//WiFiServer server(80);
//create a websocket object
AsyncWebSocket ws("/ws");

//Json variable to hold sensor readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 300;

//Initialize SPIFFS 
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  //hotspot ssid and password 
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
    Serial.print("Status: ");
    Serial.println(WiFi.status());
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println("Connected to WiFi");
}

void notifyClients(String sensorReadings) {
  ws.textAll(sensorReadings);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    //data[len] = 0;
    //String message = (char*)data;
    // Check if the message is "getReadings"
    //if (strcmp((char*)data, "getReadings") == 0) {
      //if it is, send current sensor readings
      String sensorReadings = getSensorReadings();
      Serial.print(sensorReadings);
      // send to data A&A function 
      notifyClients(sensorReadings);
    //}
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

unsigned long time_since_last_reset = 0;  
int interval_one = 3000;    

float getRPM(int RPM_type) {
  // start sensor calculations 
  // preallocate values for tach
  while (true){
    bool on_state = false;
    time_since_last_reset = millis(); 
    // counting number of times the hall sensor is tripped
    // but without double counting during the same trip
    float rpm_val = 0;
    while((millis() - time_since_last_reset) < interval_one){ //reading for a second
      if (digitalRead(hall_pin)==1){
        if (on_state == false) {
          on_state = true; 
          rpm_val+= 1; 
        }
      }
      else{
        on_state = false;
      }
    }
  Serial.print("RPM"); 
  Serial.println(rpm_val); 
  //Serial.println(digitalRead(hall_pin));
  //Serial.println(" RPM");
  delay(1);        // delay in between reads for stability
  return rpm_val; 
  break; 
  }
  
}

String getSensorReadings(){
  readings["RIGHT RPM"] = String(getRPM(hall_pin));
  readings["LEFT RPM"] = String(getRPM(hall_pin2));
  String jsonString = JSON.stringify(readings);
  Serial.println("JSON STRING");
  Serial.println(jsonString);
  return jsonString;
}

void setup() {
  Serial.begin(115200);
  initWiFi();
  initSPIFFS();
  initWebSocket();
  pinMode(hall_pin, INPUT); 
  pinMode(hall_pin2, INPUT); 

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // send html stuff to their server 
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.serveStatic("/", SPIFFS, "/");

  // Start server
  server.begin();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    String sensorReadings = getSensorReadings();
    Serial.print(sensorReadings);
    notifyClients(sensorReadings);

  lastTime = millis();

  }
  ws.cleanupClients();
}
