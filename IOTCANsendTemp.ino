#include <ESP32CAN.h>
#include <CAN_config.h>

//#define DHpin  A8 // ESP32 pin GPIO21 connected to DHT11 sensor
/* the variable name CAN_cfg is fixed, do not change */
CAN_device_t CAN_cfg;
int DHpin = A8;


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
}

void loop() {
    CAN_frame_t rx_frame;
    unsigned int temp = analogRead(DHpin);
    temp = (byte)temp;
    uint8_t digit1 = temp / 100;
    uint8_t digit2 = (temp / 10) % 10;
    uint8_t digit3 = temp % 10;
    //receive next CAN frame from queue
    if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE){
      if(rx_frame.FIR.B.FF==CAN_frame_std){
        //printf("New standard frame");
        printf("received from other node");
      }
      else {
        //printf("New extended frame");
        printf("received from other node");
      }
      if(rx_frame.FIR.B.RTR==CAN_RTR){
        printf(" RTR from 0x%08x, DLC %d\r\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
      }
      else{
        printf(" from 0x%08x, DLC %d\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
      }
      //for(int i = 0; i < rx_frame.FIR.B.DLC; i++){
      char str[rx_frame.FIR.B.DLC];
      for(int i = 0; i < 3; i++){ 
        //printf("%c\t", (char)rx_frame.data.u8[i]);
        Serial.println(rx_frame.data.u8[i]);
        //sprintf(str, "%d", rx_frame.data.u8[i]);
      }
      //make function to concat all the separate ints into one in and put it here 
      //Serial.println("data Recieved");
      //printf("%c\t",dataReceived);
    }
    //next: put this data together into one integer 
    else {
      //if not receiving, we send data 
      //do this simultaneously? 
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
}

//int conversion(uint_8 data) {
//  // convert unsigned int to just an integer 
//  // modulo the number into separate digits 
//  // put the digits into an integer array 
//}
//      rx_frame.data.u8[2] = 'l';
//      rx_frame.data.u8[3] = 'l';
//      rx_frame.data.u8[4] = 'o';
//      rx_frame.data.u8[5] = 'c';
//      rx_frame.data.u8[6] = 'a';
//      rx_frame.data.u8[7] = 'n';
