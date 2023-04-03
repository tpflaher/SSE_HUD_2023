#include <Servo.h>  // Include the Servo library

Servo throttle;   // Create a Servo object for the throttle
int potpin = A0;  // Connect the potentiometer to pin A0
int val;          // Variable to store the potentiometer value
unsigned long prevtime;

void setup() {
 
  pinMode(potpin, INPUT); // Set the potentiometer pin as an input
pinMode(9,OUTPUT);  
  Serial.begin(9600); 
  prevtime = millis();
}

void loop() { 
  throttle.attach(9);  // Attach the servo to pin 9
  val = analogRead(potpin);
  if(val>230){  // Read the potentiometer value
    val = map(val, 230, 879, 50, 106);  // Convert the value to a range of 0-180 (throttle range)
     //Exponential rate of change
    //val = pow(val, 2) / (180 * 180); // makes the values exponential and then puts it back into the range
     throttle.write(val);
    // Set the throttle position
  } else{
    val=50;
    throttle.write(val);
 } 
   if( millis()- prevtime>= 100){
   Serial.println(val);
    prevtime = millis();
 }  
}
  



