//==================================================================================//

#include <CAN.h>
#include <DHT.h>
#define DHT_SENSOR_PIN  21 // ESP32 pin GPIO21 connected to DHT11 sensor
#define DHT_SENSOR_TYPE DHT11
#define TX_GPIO_NUM   5  // Connects to CTX
#define RX_GPIO_NUM   4  // Connects to CRX
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

//==================================================================================//

void setup() {
  Serial.begin (115200);
  //dht_sensor.begin(); // initialize the DHT sensor
  while (!Serial);
  delay (1000);
  Serial.println ("CAN SENDER");

  // Set the pins
  CAN.setPins (RX_GPIO_NUM, TX_GPIO_NUM);

  // start the CAN bus at 500 kbps
  if (!CAN.begin (500E3)) {
    Serial.println ("Starting CAN failed!");
    while (1);
  }
  else {
    Serial.println ("CAN Initialized");
  }
}

//==================================================================================//

void loop() {
  //canSender();
  //canReceiver();
  canSendTemp();
}

//==================================================================================//
void canSender() {
  // send packet: id is 11 bits, packet can contain up to 8 bytes of data
  Serial.print ("Sending packet ... ");

  CAN.beginPacket (0x12);  //sets the ID and clears the transmit buffer
  // CAN.beginExtendedPacket(0xabcdef);
  CAN.write ('1'); //write data to buffer. data is not sent until endPacket() is called.
  CAN.write ('2');
  CAN.write ('3');
  CAN.write ('4');
  CAN.write ('5');
  CAN.write ('6');
  CAN.write ('7');
  CAN.write ('8');
  CAN.endPacket();

  //RTR packet with a requested data length
  CAN.beginPacket (0x12, 3, true);
  CAN.endPacket();

  Serial.println ("done");

  //delay (1000);
  delay(1000);
}
void canSendTemp() {
  // from temp sensor:
  // read humidity 
  int humi  = dht_sensor.readHumidity();
  // read temperature in Celsius
  int tempC = dht_sensor.readTemperature();
  // read temperature in Fahrenheit
  int tempF = dht_sensor.readTemperature(true);
  // check whether the reading is successful or not
  if ( isnan(tempC) || isnan(tempF) || isnan(humi)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    // send packet: id is 11 bits, packet can contain up to 8 bytes of data
     // CAN.beginExtendedPacket(0xabcdef);
    Serial.print ("Sending packet ... ");
    CAN.beginPacket (0x12);  //sets the ID and clears the transmit buffer
   // CAN.write("Humidity: "); //write data to buffer. data is not sent until endPacket() is called.
    CAN.write(humi);
    Serial.println(humi); 
    //CAN.write("%");
   // CAN.write("  |  ");
    //CAN.write("Temperature: ");
    CAN.write(tempC);
   // CAN.write("°C  ~  ");
    CAN.write(tempF);
    //CAN.write("°F");
    CAN.endPacket();
  }
  //RTR packet with a requested data length
  CAN.beginPacket (0x12, 3, true);
  CAN.endPacket();

  Serial.println ("done");

  delay (1000);
}

//==================================================================================//
//temp sensor code isolated: 
//#include <DHT.h>
//#define DHT_SENSOR_PIN  21 // ESP32 pin GPIO21 connected to DHT11 sensor
//#define DHT_SENSOR_TYPE DHT11
//DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
//void setup() {
//  Serial.begin(9600);
//  dht_sensor.begin(); // initialize the DHT sensor
//}
//void loop() {
//  // read humidity
//  float humi  = dht_sensor.readHumidity();
//  // read temperature in Celsius
//  float tempC = dht_sensor.readTemperature();
//  // read temperature in Fahrenheit
//  float tempF = dht_sensor.readTemperature(true);
//  // check whether the reading is successful or not
//  if ( isnan(tempC) || isnan(tempF) || isnan(humi)) {
//    Serial.println("Failed to read from DHT sensor!");
//  } else {
//    Serial.print("Humidity: ");
//    Serial.print(humi);
//    Serial.print("%");
//    Serial.print("  |  ");
//    Serial.print("Temperature: ");
//    Serial.print(tempC);
//    Serial.print("°C  ~  ");
//    Serial.print(tempF);
//    Serial.println("°F");
//  }
//  // wait a 2 seconds between readings
//  delay(2000);
//}
//=================================================================================//
