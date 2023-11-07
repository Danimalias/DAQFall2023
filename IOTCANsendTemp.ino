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
  // always collecting Temp data 
  collectTemp();
  //receive next CAN frame from queue
  CAN_frame_t rx_frame;
  rx_frame.FIR.B.FF = CAN_frame_std;
  rx_frame.MsgID = 2;
  rx_frame.FIR.B.DLC = 8;
//  if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE){
//    receiveData();
//  }
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
        //Serial.println(rx_frame.data.u8[i]);
        //sprintf(str, "%d", rx_frame.data.u8[i]);
      }
      int x = rx_frame.data.u8[0];
      int y = rx_frame.data.u8[1];
      int z = rx_frame.data.u8[2];
      printf("%d%d%d",x,y,z);
      //make function to concat all the separate ints into one in and put it here 
      //Serial.println("data Recieved"); 
      //printf("%c\t",dataReceived);
    }
   else {
    //sendTemp();
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

void receiveData(){
  CAN_frame_t rx_frame;
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
  // print received data to terminal 
  int x = rx_frame.data.u8[0];
  int y = rx_frame.data.u8[1];
  int z = rx_frame.data.u8[2];
  // concat digits into one integer
  Serial.println("Received: ");
  //Serial.println(x,y,z);
  Serial.println(x);
  Serial.println(y);
  Serial.println(z);
  //printf("%d%d%d",x,y,z);
}
