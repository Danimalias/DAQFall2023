////// setting up the CAN driver 
////
//#include <driver/can.h>
//#include <driver/gpio.h>
//
//void setup() { 
//  // put your setup code here, to run once:
// // connect to GPIO 4 and 5 
// Serial.begin(115200);
// setup_can_driver();
//}
//
//void setup_can_driver() {
//  // put your main code here, to run repeatedly:
//  can_general_config_t general_config={
//    .mode = CAN_MODE_NORMAL,
//    .tx_io = (gpio_num_t)GPIO_NUM_5,
//    .rx_io = (gpio_num_t)GPIO_NUM_4,
//    .clkout_io = (gpio_num_t)CAN_IO_UNUSED,
//    .bus_off_io = (gpio_num_t)CAN_IO_UNUSED,
//    .tx_queue_len = 0, //not transmitting rn 
//    .rx_queue_len = 65,
//    .alerts_enabled = CAN_ALERT_ALL,
//    .clkout_divider =  0
//  };
//  can_timing_config_t timing_config = CAN_TIMING_CONFIG_250KBITS();
//  can_filter_config_t filter_config = CAN_FILTER_CONFIG_ACCEPT_ALL();
//  esp_err_t error;
//  error = can_driver_install(&general_config, &timing_config, &filter_config);
//
//  if (error==ESP_OK){
//    Serial.println("CAN Driver Installation ok");
//  }
//  else{
//    Serial.println("CAN Driver installation failed");
//    return;
//  }
//  error = can_start();
//
//  if(error==ESP_OK){
//    Serial.println("CAN Driver started");
//  }
//  else{
//    Serial.println("CAN driver failed to start");
//    return;
//  }
//}
//void loop(){
//  can_message_t rx_frame;
//  if(can_receive(&rx_frame,pdMS_TO_TICKS(1000))==ESP_OK)
//  {
//    printf("from 0x%08X, DLC%d,DATA", rx_frame.identifier,rx_frame.data_length_code);
//    for(int i=0; i<rx_frame.data_length_code;i++){
//      printf(" 0x%02X ", rx_frame.data[i]);
//    }
//    printf("\n");
//  }
//}
////
//// DIFFERENT SCRIPT BELOW
//

// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <CAN.h>

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("CAN Receiver");

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
}

void loop() {
  // try to parse packet
  int packetSize = CAN.parsePacket();

  if (packetSize || CAN.packetId() != -1) {
    // received a packet
    Serial.print("Received ");

    if (CAN.packetExtended()) {
      Serial.print("extended ");
    }

    if (CAN.packetRtr()) {
      // Remote transmission request, packet contains no data
      Serial.print("RTR ");
    }

    Serial.print("packet with id 0x");
    Serial.print(CAN.packetId(), HEX);

    if (CAN.packetRtr()) {
      Serial.print(" and requested length ");
      Serial.println(CAN.packetDlc());
    } else {
      Serial.print(" and length ");
      Serial.println(packetSize);

      // only print packet data for non-RTR packets
      while (CAN.available()) {
        Serial.print((char)CAN.read());
      }
      Serial.println();
    }

    Serial.println();
  }
}
