#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <CAN.h>
#include <ESP32CAN.h>
#include <CAN_config.h>
#include <ArduinoJson.h>

CAN_device_t CAN_cfg;


const char* ssid = "name";
const char* password = "password";
const char* websocket_server_host = "192.186.0.1";
const uint16_t websocket_server_port = 8080;

using namespace websockets;

WebsocketsClient client;



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


void setup() {
        Serial.begin(115200);
    Serial.println("iotsharing.com CAN demo");
    /* set CAN pins and baudrate */
    CAN_cfg.speed=CAN_SPEED_1000KBPS;
    CAN_cfg.tx_pin_id = GPIO_NUM_5;
    CAN_cfg.rx_pin_id = GPIO_NUM_4;
    /* create a queue for CAN receiving */ 
    CAN_cfg.rx_queue = xQueueCreate(10,sizeof(CAN_frame_t));
    //initialize CAN Module
    ESP32Can.CANInit();

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting...");
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

void sendBack(){
    CAN_frame_t rx_frame;
    //receive next CAN frame from queue
    if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE){
      if(rx_frame.FIR.B.RTR == CAN_RTR){
            char buffer[50];
            snprintf(buffer, sizeof(buffer), "RTR from 0x%08x, DLC %d\r\n", rx_frame.MsgID, rx_frame.FIR.B.DLC);
            client.send(buffer);
        }
      else{
        String dataMessage;
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "Data from 0x%08x, DLC %d: ", rx_frame.MsgID, rx_frame.FIR.B.DLC);
        dataMessage += buffer;

        for (int i = 0; i < rx_frame.FIR.B.DLC; i++) {
            char byteStr[4];
            snprintf(byteStr, sizeof(byteStr), "%02X ", rx_frame.data.u8[i]);
            dataMessage += byteStr;
        }
        client.send(dataMessage.c_str());
        }
      }
      //ESP32Can.CANWriteFrame(&rx_frame);
    }
