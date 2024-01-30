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


/* the variable name CAN_cfg is fixed, do not change */
CAN_device_t CAN_cfg;

void setup() {
  Serial.begin(115200);
  pinMode(hall_pin, INPUT);               // make the hall pin an input
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
  CAN_frame_t rx_frame2;     
      
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

    // start RPM sensor calculation 
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
//  Serial.print("Time Passed: ");
//  Serial.print(total_time);
//  Serial.println("s");
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
  }
  Serial.println();
  
  delay(1);        // delay in between reads for stability

//    // CAN for joystick 
//    rx_frame.FIR.B.FF = CAN_frame_std;
//    rx_frame.MsgID = 2;
//    rx_frame.FIR.B.DLC = 8;
//    rx_frame.data.u8[0] = char(digity0);
//    rx_frame.data.u8[1] = char(digity1);
//    rx_frame.data.u8[2] = char(digity2);
//    ESP32Can.CANWriteFrame(&rx_frame);
    //CAN for RPM 
    rx_frame2.MsgID = 3; 
    rx_frame2.FIR.B.DLC = 8; 
    rx_frame2.data.u8[0] = char(digits[0]);
    rx_frame2.data.u8[1] = char(digits[1]);
    rx_frame2.data.u8[2] = char(digits[2]);
    rx_frame2.data.u8[3] = char(digits[3]);
    rx_frame2.data.u8[4] = char(digits[4]); 
    ESP32Can.CANWriteFrame(&rx_frame2); 

  }
}
