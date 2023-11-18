#include <ESP32CAN.h>
#include <CAN_config.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <CAN.h>
#include <ArduinoJson.h>
/* the variable name CAN_cfg is fixed, do not change */

CAN_device_t CAN_cfg;


const char* ssid = "Hi";
const char* password = "edwardiscool";
const char* websocket_server_host = "192.168.48.229";
const uint16_t websocket_server_port = 8080;

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
            sendBack();
        }
    }
  
}



void sendBack() {
  CAN_frame_t rx_frame;
  //receive next CAN frame from queue
  // if it received something, send it back
    if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE){
    if(rx_frame.FIR.B.RTR==CAN_RTR){
      char buffer[50];
      snprintf(buffer, sizeof(buffer), " RTR from 0x%08x, DLC %d\r\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
      client.send(buffer);
    }
    else{
      String dataMessage;
      char buffer[50];
      snprintf(buffer, sizeof(buffer), " from 0x%08x, DLC %d\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
      dataMessage += buffer;
      
      for(int i = 0; i < rx_frame.FIR.B.DLC; i++){
        char byteStr[4];
        snprintf(byteStr, sizeof(byteStr), "%c\t", (char)rx_frame.data.u8[i]);
        dataMessage += byteStr;
      }
      printf("\n");
    }
  }
  else{
    Serial.println("--reached else going to send joystick");
    //sendJoystick();
    // read X and Y analog values
    unsigned int valueX = analogRead(VRX_PIN);
    unsigned int valueY = analogRead(VRY_PIN);
    valueX = byte(valueX); 
    valueY = byte(valueY); 
    
    uint8_t digitx0 = valueX / 100;
    uint8_t digitx1 = (valueX / 10) % 10;
    uint8_t digitx2 = valueX % 10;
  
    uint8_t digity0 = valueY / 100;
    uint8_t digity1 = (valueY / 10) % 10;
    uint8_t digity2 = valueY % 10;
    
    client.send("x = " + String(valueX));
    client.send("y = " + String(valueY));
    rx_frame.FIR.B.FF = CAN_frame_std;
    rx_frame.MsgID = 2;
    rx_frame.FIR.B.DLC = 8;
    rx_frame.data.u8[0] = char(digitx0);
    rx_frame.data.u8[1] = char(digitx1);
    rx_frame.data.u8[2] = char(digitx2);
    rx_frame.data.u8[4] = char(digity0);
    rx_frame.data.u8[5] = char(digity1);
    rx_frame.data.u8[6] = char(digity2);
    ESP32Can.CANWriteFrame(&rx_frame);
  }
}
 
void sendJoystick() {
  CAN_frame_t rx_frame;
  // read X and Y analog values
  unsigned int valueX = analogRead(VRX_PIN);
  unsigned int valueY = analogRead(VRY_PIN);
  valueX = byte(valueX); 
  valueY = byte(valueY); 
  
  uint8_t digitx0 = valueX / 100;
  uint8_t digitx1 = (valueX / 10) % 10;
  uint8_t digitx2 = valueX % 10;

  uint8_t digity0 = valueY / 100;
  uint8_t digity1 = (valueY / 10) % 10;
  uint8_t digity2 = valueY % 10;
  
  Serial.print("x = ");
  Serial.println(valueX);
  Serial.print(", y = ");
  Serial.println(valueY);
  rx_frame.FIR.B.FF = CAN_frame_std;
  rx_frame.MsgID = 2;
  rx_frame.FIR.B.DLC = 8;
  rx_frame.data.u8[0] = char(digitx0);
  rx_frame.data.u8[1] = char(digitx1);
  rx_frame.data.u8[2] = char(digitx2);
  rx_frame.data.u8[4] = char(digity0);
  rx_frame.data.u8[5] = char(digity1);
  rx_frame.data.u8[6] = char(digity2);
  ESP32Can.CANWriteFrame(&rx_frame);
}
