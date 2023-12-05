#include <Arduino.h>
#include <ESP32CAN.h>
#include <CAN_config.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <CAN.h>
#include <ArduinoJson.h> 
#include <stdio.h>
/* the variable name CAN_cfg is fixed, do not change */

CAN_device_t CAN_cfg;

#define DHpin  A8     // ESP32 pin GPIO21 connected to DHT11 sensor for temperature

void sendTemp();
void collectTemp();

const char* ssid = "Dan";
const char* password = "muanha21";
// this IP address is what outputted from node server.js 
const char* websocket_server_host = "172.20.10.8";
const uint16_t websocket_server_port = 8080;
const char* msg = NULL;
const char* msg1 = NULL;


using namespace websockets;

WebsocketsClient client;

void sendBack();


void onMessageCallback(WebsocketsMessage message) {
    Serial.print("Got Message: ");
    Serial.println(message.data());
}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}

//A0 is 26 A1 is 25
#define VRX_PIN  26 // ESP32 pin GPIO36 (ADC0) connected to VRX pin
#define VRY_PIN  25 // ESP32 pin GPIO39 (ADC0) connected to VRY pin

int valueX = 0; // to store the X-axis value
int valueY = 0; // to store the Y-axis value

void setup() {
  Serial.begin(115200);
  Serial.println("iotsharing.com CAN demo");
  /* set CAN pins and baudrate */
  CAN_cfg.speed = CAN_SPEED_1000KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_5;
  CAN_cfg.rx_pin_id = GPIO_NUM_4;
  /* create a queue for CAN receiving */
  CAN_cfg.rx_queue = xQueueCreate(10, sizeof(CAN_frame_t));
  //initialize CAN Module
  ESP32Can.CANInit();

  WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting...");
        Serial.print("Status: ");
        Serial.println(WiFi.status());
    }
    Serial.println("Connected to WiFi");

    client.onMessage(onMessageCallback);
    client.onEvent(onEventsCallback);
    client.connect(websocket_server_host, websocket_server_port, "/");
}


void loop() {
client.poll();
    if (client.available()) {
        static unsigned long lastTime = 0;
        unsigned long currentTime = millis();
        if (currentTime - lastTime > 1000) {
            lastTime = currentTime;
            client.send("Hello");
            // always collecting Temp data 
            //collectTemp();
            // receive next CAN frame from queue 
            // this is for temp data being sent back to 
            CAN_frame_t rx_frame;
            rx_frame.FIR.B.FF = CAN_frame_std;
            rx_frame.MsgID = 1;
            rx_frame.FIR.B.DLC = 8;
          
            // if receive something send it back 
            if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE){
              if(rx_frame.FIR.B.FF==CAN_frame_std){
                //printf("New standard frame");
                printf("Standard Frame and received from other node");
              }
              else {
                //printf("New extended frame");
                printf("Extended Frame and received from other node");
              }
              if(rx_frame.FIR.B.RTR==CAN_RTR){
                printf(" RTR from 0x%08x, DLC %d\r\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
              }
              else{
                printf(" from 0x%08x, DLC %d\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
              }
              
              // print data received: it could be from joystick or RPM 
              if (rx_frame.MsgID == 2) {
                int joystick1 = rx_frame.data.u8[0]; 
                int joystick2 = rx_frame.data.u8[1]; 
                int joystick3 = rx_frame.data.u8[2]; 
                int joystick4 = rx_frame.data.u8[3]; 
                int joystick5 = rx_frame.data.u8[4]; 
                int joystick6 = rx_frame.data.u8[5]; 
                Serial.println("Received Data"); 
                printf("%d%d%d%d%d%d", joystick1, joystick2, joystick3, joystick4, 
                  joystick5, joystick6); 
                client.send("joystick1 test");
                std::string strNum1 = std::to_string(joystick1);
                std::string strNum2 = std::to_string(joystick2);               
                std::string strNum3 = std::to_string(joystick3);
                std::string strNum4 = std::to_string(joystick4);
                std::string strNum5 = std::to_string(joystick5);
                std::string strNum6 = std::to_string(joystick6);
                std::string xjoystick = strNum1+strNum2+strNum3;
                std::string yjoystick = strNum4+strNum5+strNum6;
                //int var = rx_frame.data.u8;
                //below works
                msg = xjoystick.c_str();
                msg1 = yjoystick.c_str();
                //msg = std::to_string(xjoystick).c_str();
                
                client.send(msg);
                Serial.println(" "); 
                client.send(msg1);
              } else if (rx_frame.MsgID == 3) {
                int data1 = rx_frame.data.u8[0]; 
                int data2 = rx_frame.data.u8[1]; 
                int data3 = rx_frame.data.u8[2]; 
                int data4 = rx_frame.data.u8[3]; 
                int data5 = rx_frame.data.u8[4]; 
                Serial.println("Received Data"); 
                printf("%d%d%d%d%d%d", data1, data2, data3, data4, data5); 
                Serial.println(" "); 
              } else {
                Serial.println("Received not from a recognized frame"); 
              } 
          }
        }
    }
  
}

void sendTemp(){
  unsigned int temp = analogRead(DHpin);
  temp = (byte)temp;
  uint8_t digit1 = temp / 100;
  uint8_t digit2 = (temp / 10) % 10;
  uint8_t digit3 = temp % 10;
  CAN_frame_t rx_frame;
  rx_frame.FIR.B.FF = CAN_frame_std;
  rx_frame.MsgID = 1;
  rx_frame.FIR.B.DLC = 8;
  rx_frame.data.u8[0] = char(digit1); 
  rx_frame.data.u8[1] = char(digit2); 
  rx_frame.data.u8[2] = char(digit3); 
  Serial.println("-------temp-------");
  Serial.println(temp);
  Serial.println("-------digit1-------");
  Serial.println(digit1);
  Serial.println("-------digit2-------");
  Serial.println(digit2);
  Serial.println("-------digit3-------");
  Serial.println(digit3);
  ESP32Can.CANWriteFrame(&rx_frame);
  //delay(200);
}
void collectTemp(){
  unsigned int temp = analogRead(DHpin);
  Serial.println("-------temp-------");
  Serial.println(temp);
  Serial.println("-------end temp-------");
}
