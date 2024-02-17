#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
// spiffs makes the file system 
#include "SPIFFS.h"
#include <Arduino_JSON.h>

#include <ESP32CAN.h>
#include <CAN.h>
#include <CAN_config.h>
#include <stdio.h>

//TAKEN FROM https://randomnerdtutorials.com/esp32-websocket-server-sensor/

// Potentiometer Instantiations
#define VRY_PIN  25 // ESP32 pin A1 connected to VRY pin

// RPM INSTANTIATIONS 
int hall_pin = 27;        // digital pin 2 is the hall pin

// CAN INSTANTIATIONS
CAN_device_t CAN_cfg;

const char* ssid = "Dionna";
const char* password = "dionnaiscool";

// creating an AsyncWebServer object on port 80
AsyncWebServer server(80);
//create a websocket object
AsyncWebSocket ws("/ws");

//Json variable to hold sensor readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 300;

// init sensors/ CAN here
void CAN_initalize(){
  CAN_frame_t rx_frame;
  // if receive something send it back 
  if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE){
    if(rx_frame.FIR.B.FF==CAN_frame_std){
      printf("Standard Frame and received from other node");
    }
    else {
      printf("Extended Frame and received from other node");
    }
    if(rx_frame.FIR.B.RTR==CAN_RTR){
      printf(" RTR from 0x%08x, DLC %d\r\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
    }
    else{
      printf(" from 0x%08x, DLC %d\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
    }
    // print data received: it is from the temperature sensor for now 
    if (rx_frame.MsgID == 3) {
      int data1 = rx_frame.data.u8[0]; 
      int data2 = rx_frame.data.u8[1]; 
      int data3 = rx_frame.data.u8[2]; 
      int data4 = rx_frame.data.u8[3]; 
      int data5 = rx_frame.data.u8[4]; 
      Serial.println("Received Data"); 
      printf("%d%d%d%d%d%d\n", data1, data2, data3, data4, data5); 
      std::string strNum1 = std::to_string(data1);
      std::string strNum2 = std::to_string(data2);               
      std::string strNum3 = std::to_string(data3);
      std::string strNum4 = std::to_string(data4);
      std::string strNum5 = std::to_string(data5);

      std::string temper = strNum1+strNum2+strNum3+strNum4+strNum5;
      //msg1 = temper.c_str(); 
    }
    else {
      Serial.println("Received not from a recognized frame");
    }
  }
}

// get sensor data here 

int get_joystick(){
  int valueY = 0; // to store the Y-axis value
  valueY = analogRead(VRY_PIN);
  return valueY; 
}

float get_rpm() {
  // start RPM sensor calculation
  int hall_count = 0;
  float total_time = 0;     // set number of hall trips for RPM reading (higher improves accuracy)
  if(total_time > 1){
     total_time = 0;
     hall_count = 0;
  }
  // preallocate values for tach
  float start = micros();
  bool on_state = false;
  float rpmVal = 0;
  
  // counting number of times the hall sensor is tripped
  // but without double counting during the same trip
  while(true){
    if (digitalRead(hall_pin)==1){
      if (on_state==false){
        on_state = true;
        hall_count+=1;
      }
    }
    else{
      on_state = false;
      break;
    }
  }
  // print information about Time and RPM
  float end_time = micros();
  total_time = total_time+((end_time-start)/1000000.0);
  float rpm_val = ((float)hall_count/total_time)*60.0;
  // make the rpm value 0 if its NaN, or else prints out 2147483647
  bool nan = isnan(rpm_val);
  if (nan) {
    Serial.println("RPM value is NaN");
    rpm_val = 0;
  }
  Serial.print("RPM_val: "); 
  Serial.println(rpm_val); 
  delay(1);        // delay in between reads for stability
  return rpm_val;
 }

// will need left rpm eventually 
//get sensor readings and return JSON object 

String getSensorReadings(){
  readings["RPM"] = String(get_rpm());
  readings["Joystick"] = String(get_joystick());
  String jsonString = JSON.stringify(readings);
  Serial.println("JSON STRING");
  Serial.println(jsonString);
  return jsonString;
}


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

void setup() {
  Serial.begin(115200);
  initWiFi();
  initSPIFFS();
  initWebSocket();

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
  //ws.cleanupClients();
}
