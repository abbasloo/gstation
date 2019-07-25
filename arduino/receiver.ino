//ubuntu commands
//sudo usermod -a -G dialout lolo
//udo chmod a+rw /dev/ttyACM0

//a toy model for the receiver

#define PI 3.1415926535897932384626433832795

void setup() {
  // initialize serial
  Serial.begin(9600);
}

void loop() {
   int D = 100;
   for (int i = 1; i <= D; i++){
      if (i%2 == 0){
        Serial.print("$GPGGA,145416.00,3350.10959,N,11751.22870,W,1,09,0.85,70.3,M,-32.7,M,,*5B");
        Serial.print("\n");
        Serial.flush();
        delay(1000);        
      }else{
        Serial.print(sin(2*PI/D*i));
        Serial.print(";");
        Serial.flush();
        delay(1000);
      }
   }
   delay(1000);
}
