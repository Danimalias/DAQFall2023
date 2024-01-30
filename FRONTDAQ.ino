#include <Arduino.h>
#include <ESP32CAN.h>
#include <CAN_config.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <CAN.h>
//#include <ArduinoJson.h> 
#include <stdio.h>

// ************* VARIABLE INSTANTIATIONS **************//
// **************PINOUTS*******************************//
// ****************************************************//

//A0 is 26 A1 is 25

// Button
const int buttonPin = 25;        
int buttonState = 0; 

void sendBack();

// RPM 1 INSTANTIATIONS 
int digits[5];            // int array to store the rpm value
int hall_pin = 13;        // digital pin 2 is the hall pin
float total_time = 0;     // set number of hall trips for RPM reading (higher improves accuracy)
int hall_count = 0;

/* the variable name CAN_cfg is fixed, do not change */
CAN_device_t CAN_cfg;

// websocket set up 
const char* ssid = "Dan";
const char* password = "muanha21";
// this IP address is what outputted from node server.js 
const char* websocket_server_host = "172.20.10.8";
const uint16_t websocket_server_port = 8080;
const char* msg = NULL;
const char* msg1 = NULL;

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
  pinMode(hall_pin, INPUT);               // make the hall pin an input
  pinMode(buttonPin, INPUT);              // make button an input 
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
   CAN_frame_t rx_frame;
   
  int buttonState = digitalRead(buttonPin); 
  printf("Button state: %d", buttonState); 
  client.poll();
    if (client.available()) {
        static unsigned long lastTime = 0;
        unsigned long currentTime = millis();
        if (currentTime - lastTime > 1000) {
            lastTime = currentTime;
            
            // send button data via websocket 
            client.send("Button State");
            std::string strNum1 = std::to_string(buttonState);
            msg = strNum1.c_str();
            client.send(msg);

            // send RIGHT RPM data via websocket 
//            int* rpmResult = RPMCalc(); 
//            printf("Right RPM values"); 
//            for (int i = 0; i < 5; ++i) {
//               printf("%d", rpmResult[i]);
//            }
//            int rpm1 = rpmResult[0]; 
//            int rpm2 = rpmResult[1]; 
//            int rpm3 = rpmResult[2]; 
//            int rpm4 = rpmResult[3]; 
//            int rpm5 = rpmResult[4]; 
//            client.send("Right RPM"); 
//            std::string rpmNum1 = std::to_string(rpm1);
//            std::string rpmNum2 = std::to_string(rpm2);
//            std::string rpmNum3 = std::to_string(rpm3);
//            std::string rpmNum4 = std::to_string(rpm4);
//            std::string rpmNum5 = std::to_string(rpm5);
//
//            std::string rpmString = rpmNum1+rpmNum2+rpmNum3+rpmNum4+rpmNum5; 
//
//            rpmmsg = rpmString.c_str(); 
//            client.send(rpmmsg); 
            
//            // free allocated memory 
//            delete[] rpmResult; 
           
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

              // print data received: it is from the second RPM sensor for now 
              if (rx_frame.MsgID == 3) {
                int data1 = rx_frame.data.u8[0]; 
                int data2 = rx_frame.data.u8[1]; 
                int data3 = rx_frame.data.u8[2]; 
                int data4 = rx_frame.data.u8[3]; 
                int data5 = rx_frame.data.u8[4]; 
                Serial.println("Received Data"); 
                printf("%d%d%d%d%d%d", data1, data2, data3, data4, data5); 
                client.send("Left RPM Data"); 
                std::string strNum1 = std::to_string(data1);
                std::string strNum2 = std::to_string(data2);               
                std::string strNum3 = std::to_string(data3);
                std::string strNum4 = std::to_string(data4);
                std::string strNum5 = std::to_string(data5);
    
                std::string leftRPM = strNum1+strNum2+strNum3+strNum4+strNum5;
                msg = leftRPM.c_str(); 
                client.send(msg); 
   
              } else {
                Serial.println("Received not from a recognized frame"); 
              } 
          }
        }
    }

}

int* RPMCalc() {
  // start RPM sensor calculation 
  int* rpm = new int[5]; 
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
       // Serial.println("on");
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
  int convert_rpm_val = rpm_val * 100;            // turns the float into an int 
  Serial.print("RPM_val: "); 
  Serial.println(rpm_val); 
  
  Serial.print("Converted RPM Val: "); 
  Serial.println(convert_rpm_val); 
  
  // convert the value into single digits and put it in an array 
  int divisor = 10000;
  int index = 0;

  while (divisor > 0) {
    int digit = convert_rpm_val / divisor;
    digits[index++] = digit;
    convert_rpm_val %= divisor;
    divisor /= 10;
  }
  // prints out the RPM values for testing 
  for (int i = 0; i < 5; i++) { // Assuming a maximum of 5 digits
    Serial.print(digits[i]);
    rpm[i] = digits[i]; 
  }
  Serial.println();
  delay(1);        // delay in between reads for stability
  return rpm; 

 }
