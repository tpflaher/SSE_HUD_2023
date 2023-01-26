#include <Servo.h>  // Include the Servo library

Servo throttle;   // Create a Servo object for the throttle
int potpin = A0;  // Connect the potentiometer to pin A0
int val;          // Variable to store the potentiometer value

void setup() {
  throttle.attach(9);  // Attach the servo to pin 9
  pinMode(potpin, INPUT);  // Set the potentiometer pin as an input
}

void loop() {
  val = analogRead(potpin);  // Read the potentiometer value
  val = map(val, 0, 1023, 0, 180);  // Convert the value to a range of 0-180 (throttle range)
  // Exponential rate of change
  val = pow(val, 2) / (180 * 180); // makes the values exponential and then puts it back into the range
  throttle.write(val);  // Set the throttle position
}
  

