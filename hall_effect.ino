// digital pin 2 is the hall pin
int hall_pin = 7;
// set number of hall trips for RPM reading (higher improves accuracy)
float total_time = 0;
int hall_count = 0;


void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
  // make the hall pin an input:
  pinMode(hall_pin, INPUT);
}

// the loop routine runs over and over again forever:
void loop() {
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