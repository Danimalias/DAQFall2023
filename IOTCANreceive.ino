#include <ESP32CAN.h>
#include <CAN_config.h>
/* the variable name CAN_cfg is fixed, do not change */

//A0 is 26 A1 is 25
#define VRX_PIN  26 // ESP32 pin GPIO36 (ADC0) connected to VRX pin
#define VRY_PIN  25 // ESP32 pin GPIO39 (ADC0) connected to VRY pin

int valueX = 0; // to store the X-axis value
int valueY = 0; // to store the Y-axis value

// rpm stuff
// digital pin 2 is the hall pin
int hall_pin = 13;
// set number of hall trips for RPM reading (higher improves accuracy)
float total_time = 0;
int hall_count = 0;


CAN_device_t CAN_cfg;
void setup() {
  Serial.begin(115200);
  Serial.println("iotsharing.com CAN demo");
  // make the hall pin an input:
  pinMode(hall_pin, INPUT);
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
  //sendJoystick();
  rpm(); 
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
    rx_frame.data.u8[3] = char(digity0);
    rx_frame.data.u8[4] = char(digity1);
    rx_frame.data.u8[5] = char(digity2);

    Serial.println("Sending RPM Data: 0/1 and Time Elapsed"); 
    rpm(); 
    
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

void rpm () {
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
  Serial.print("Time Passed: ");
  Serial.print(total_time);
  Serial.println("s");
  float rpm_val = ((float)hall_count/total_time)*60.0;
  Serial.println(digitalRead(hall_pin));
  Serial.println(" RPM");
  delay(1);        // delay in between reads for stability
}
