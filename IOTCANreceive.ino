#include <ESP32CAN.h>
#include <CAN_config.h>
/* the variable name CAN_cfg is fixed, do not change */
CAN_device_t CAN_cfg;
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
  sendBack();
//    CAN_frame_t rx_frame;
//    //receive next CAN frame from queue
//    if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE){
//      //do stuff!
//      if(rx_frame.FIR.B.FF==CAN_frame_std)
//        printf("New standard frame");
//      else
//        printf("New extended frame");
//      if(rx_frame.FIR.B.RTR==CAN_RTR)
//        printf(" RTR from 0x%08x, DLC %d\r\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
//      else{
//        printf(" from 0x%08x, DLC %d\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
//        //convert to upper case and respond to sender
//        for(int i = 0; i < rx_frame.FIR.B.DLC; i++){
//          if(rx_frame.data.u8[i] >= '0' && rx_frame.data.u8[i] <= '9'){
//            rx_frame.data.u8[i] = rx_frame.data.u8[i];
//            Serial.println("-----i------");
//            Serial.println(rx_frame.data.u8[i]);
//            ESP32Can.CANWriteFrame(&rx_frame);
//          }
//        }
//      }
//      for (int i = 0; i < rx_frame.FIR.B.DLC; i++) {
//        printf("0x%02X ", rx_frame.data.u8);
//      }
        //printf("\n");
      //}
      //respond to sender
   
}

void sendBack(){
    CAN_frame_t rx_frame;
    //receive next CAN frame from queue
    if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE){
      if(rx_frame.FIR.B.FF==CAN_frame_std){
        printf("New standard frame");
      }
      else{
        printf("New extended frame");
      }
      if(rx_frame.FIR.B.RTR==CAN_RTR){
        printf(" RTR from 0x%08x, DLC %d\r\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
      }
      else{
        printf(" from 0x%08x, DLC %d\n",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
      }
      for(int i = 0; i < rx_frame.FIR.B.DLC; i++){
        Serial.println(rx_frame.data.u8[i]);
        //printf(rx_frame.data.u8[i]);
        //ESP32Can.CANWriteFrame(&rx_frame);
      }
      ESP32Can.CANWriteFrame(&rx_frame);
      // rx_frame is int 14
      // &rx_frame type is CAN_frame_t but has 8 bits
//      for (int i = 0; i < rx_frame.FIR.B.DLC; i++) {
//        printf("0x%02X ", rx_frame.data.u8);
//      }
        //printf("\n");
      //}
      //respond to sender
    }
}
    
