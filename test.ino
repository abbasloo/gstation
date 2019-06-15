void setup() {
  // initialize serial:
  Serial.begin(9600);
}

void loop() {
   for (int i = 0; i <= 25; i++){
      Serial.print(i);
      Serial.print(",");
      Serial.flush();
      delay(1000);
   }
   delay(1000);
}
