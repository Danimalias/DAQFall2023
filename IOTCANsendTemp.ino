#include <ESP32CAN.h>
#include <CAN_config.h>

// ************* VARIABLE INSTANTIATIONS **************//
// **************PINOUTS*******************************//
// ****************************************************//

#define DHpin  A8     // ESP32 pin GPIO21 connected to DHT11 sensor for temperature
int DHpin = A8;

/* the variable name CAN_cfg is fixed, do not change */
CAN_device_t CAN_cfg;


void setup() {
  Serial.begin(115200);
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
      Serial.println(" "); 
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

    // Formatting Joystick Data 
//    char str[rx_frame.FIR.B.DLC];
//
//    int x = rx_frame.data.u8[0];
//    int y = rx_frame.data.u8[1];
//    int z = rx_frame.data.u8[2];
//
//    int a = rx_frame.data.u8[3];
//    int b = rx_frame.data.u8[4];
//    int c = rx_frame.data.u8[5];
    
//      printf("x values");
//      printf("%d%d%d",x,y,z);
//      printf("y values");
//      printf("%d%d%d",a,b,c);
      
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

//void receiveData(){
//  CAN_frame_t rx_frame;
//  if(rx_frame.FIR.B.FF==CAN_frame_std){
//    //printf("New standard frame");
//    printf("Standard frame and received from other node");
//  }
//  else {
//    //printf("New extended frame");
//    printf("extended frame and received from other node");
//  }
//  if(rx_frame.FIR.B.RTR==CAN_RTR){
//    printf(" RTR from 0x%08x, DLC %d\r\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
//  }
//  else{
//    printf(" from 0x%08x, DLC %d\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
//  }
//  // print received data to terminal 
//  // prints the joystick data 
////  int x = rx_frame.data.u8[0];
////  int y = rx_frame.data.u8[1];
////  int z = rx_frame.data.u8[2];
////  // concat digits into one integer
////  Serial.println("Received: ");
////  //Serial.println(x,y,z);
////  Serial.println(x);
////  Serial.println(y);
////  Serial.println(z);
//  //printf("%d%d%d",x,y,z);
//  
//}
