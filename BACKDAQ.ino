#include <Arduino.h>
#include <ESP32CAN.h>
#include <CAN_config.h>

// ************* VARIABLE INSTANTIATIONS **************// 
// **************PINOUTS*******************************//
// ****************************************************//


void sendBack();

// RPM 2 INSTANTIATIONS 
int digits[5];            // int array to store the rpm value
int hall_pin = 13;        // digital pin 2 is the hall pin
float total_time = 0;     // set number of hall trips for RPM reading (higher improves accuracy)
int hall_count = 0;
#define DHpin  A0

/* the variable name CAN_cfg is fixed, do not change */
CAN_device_t CAN_cfg;

void setup() {
  Serial.begin(115200);
  /* set CAN pins and baudrate */
  CAN_cfg.speed = CAN_SPEED_1000KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_5;
  CAN_cfg.rx_pin_id = GPIO_NUM_4;
  /* create a queue for CAN receiving */
  CAN_cfg.rx_queue = xQueueCreate(10, sizeof(CAN_frame_t));

  //initialize CAN Module
  ESP32Can.CANInit();
}

void loop() {
  sendBack();
}

void sendBack() {
  CAN_frame_t rx_frame;
  // since we are sending two things we need to add another frame
  //receive next CAN frame from queue
  // if it received something, send it back
    if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE){
    if(rx_frame.FIR.B.FF==CAN_frame_std)
      printf("New standard frame");
    else
      printf("New extended frame");
    if(rx_frame.FIR.B.RTR==CAN_RTR)
      printf(" RTR from 0x%08x, DLC %d\r\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
    else{
      printf(" from 0x%08x, DLC %d\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
      for(int i = 0; i < 8; i++){
        printf("%c\t", (char)rx_frame.data.u8[i]);
      }
      printf("\n");
    }
  }
  else{
    Serial.println("--reached");
    sendTemp();
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
  rx_frame.MsgID = 3;
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
