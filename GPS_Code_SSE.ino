#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <string.h>

TinyGPSPlus GPS; // creates the gps object
SoftwareSerial Display (12,13); // sets up the software Serial to digital pins 12 and 13

// global variables used to store data
double latitude;
double longitude;
double prevLatitude;
double prevLongitude;
int numberOfSatellites;
double speed;
double altitude;
double heading;
double distance;
double currDistBetween;

bool first_cords = true; // used to set the prevLongitude and prevLatitude to the right starting value

/**
 * Default arduino function that runs once when the arduino is first powered/reset.
 * We use this function to start serial connections to the GPS and the Display, and sometimes system out (printing).
 */
void setup() {
  Serial.begin(9600);  //used for system prints
  Serial2.begin(9600); // GPS port
  Display.begin(9600); // Display software serial
}

/**
 * Default arduino function that runs repeatedly while the arduino is on.
 */
void loop() {

  //check everything is good with the GPS before trying to run the main loop code
  //if any of these checks fail the loop returns before the rest of the code can be run
  if(Serial2.available() < 0){ // check if the GPS is available by checking if there is data in the buffer
    Serial.print("GPS unavailable\n");
    return;
  }

  GPS.encode(Serial2.read()); // sends the NHEMA sentence to be parsed

  if (not GPS.location.isValid()) { // checks if the GPS location is valid
    Serial.print("GPS Bad Location\n");
    return;
  }

  if(not GPS.location.isUpdated()){ // if the GPS data has been updated
    Serial.print("GPS Location not updated\n");
    return;
  }

  if(first_cords){ // first time protocol
    first_cords = false; // coverts it to false
    prevLatitude = GPS.location.lat(); // sets prevLatitude and prevLongitude to correct values
    prevLongitude = GPS.location.lng();
  }

  // set the GPS variables
  latitude = GPS.location.lat();
  longitude = GPS.location.lng();
  altitude = GPS.altitude.feet(); //the GPS altitude in feet
  speed = GPS.speed.mph();
  numberOfSatellites = GPS.satellites.value();
  heading = GPS.course.deg();

  //
  currDistBetween = GPS.distanceBetween(prevLatitude, prevLongitude,latitude, longitude);
  if(currDistBetween > 3) { // if the distance is above around 9ft
    distance += currDistBetween / 1610.0; // converts the meters to miles
    setDisplayVar("Distance", displayFormatted(distance));
    prevLongitude = longitude; // updates the prev values
    prevLatitude =latitude;
  }

  setDisplayVar("Speed", displayFormatted(speed));
}

/**
 * Because the display interprets doubles weird, they need to be multiplied by 10
 * and then sent as a string representation of an integer to be correctly rendered.
 */
String displayFormatted(double input) {
  // the display interprets doubles weird so they need to be multiplied by 10
  return (String) (int) (input * 10);
}

void setDisplayVar(String variable, String newValue) {
  //send the updated value to the display
  Display.print(variable + ".val=" + newValue);
  // tells the display to update info (not completely sure how...)
  Display.write(0xff);
  Display.write(0xff);
  Display.write(0xff);
}