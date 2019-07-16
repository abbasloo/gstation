//sudo usermod -a -G dialout lolo
//udo chmod a+rw /dev/ttyACM0


void setup() {
  // initialize serial:
  Serial.begin(9600);
}

void loop() {
   for (int i = 0; i <= 25; i++){
      Serial.print(i);
      Serial.print(",");
      Serial.flush();
      delay(100);
   }
   delay(1000);
}
