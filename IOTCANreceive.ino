#include <ESP32CAN.h>
#include <CAN_config.h>
/* the variable name CAN_cfg is fixed, do not change */

//A0 is 26 A1 is 25
#define VRX_PIN  26 // ESP32 pin GPIO36 (ADC0) connected to VRX pin
#define VRY_PIN  25 // ESP32 pin GPIO39 (ADC0) connected to VRY pin

int valueX = 0; // to store the X-axis value
int valueY = 0; // to store the Y-axis value

CAN_device_t CAN_cfg;
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
}
void loop() {
  sendBack();
  //sendJoystick1();
}

void sendBack() {
  CAN_frame_t rx_frame;
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
