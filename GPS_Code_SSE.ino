#include <TinyGPS++.h>
#include <SoftwareSerial.h> 
#include <string.h>

TinyGPSPlus GPS; // creates the gps object
SoftwareSerial Display (12,13); // sets up the software Serial to digital pins 12 and 13
// variables used to store and convert data
double lat;
double lng;
double prevlat;
double prevlng; 
int sats;
double speed;
double alt;
double heading;
double distance;
int dis_convert;

bool first_cords = true; // used to set the prevlng and prevlat to the right starting value
String convert = ""; // used to convert ints to srtings to be sent to the display


void setup() {  // starts the serial ports
  Serial2.begin(9600); // GPS port
  Display.begin(9600); // Display software serial
}

void loop() {
  // put your main code here, to run repeatedly:
    if(Serial2.available()>0){ // if the GPS data is availibe
      GPS.encode(Serial2.read()); // sends the NHEMA setance to be parse 
      Serial1.flush(); // clears the serial port queue to reduce overflow
      if(GPS.location.isValid()){ // checks if the gps data is valid
        if(GPS.location.isUpdated()){ // if the data has been updated
          if(first_cords){ // first time protocal
            first_cords = false; // coverts it to false
            prevlat = GPS.location.lat(); // sets the prev lat and long to correct values
            prevlng = GPS.location.lng();
          }
          lat = GPS.location.lat(); // sets the variables that are wanted
          lng = GPS.location.lng();
          alt = GPS.altitude.feet();
          speed = GPS.speed.mph();
          sats = GPS.satellites.value();
          heading = GPS.course.deg(); 
          if(GPS.distanceBetween(prevlat,prevlng,lat,lng)>3){ // if the distance is above around 9ft
             distance += (GPS.distanceBetween(prevlat,prevlng,lat,lng)/1610.0); // converts the meters to miles            
          }
          // writes the 
          speed = (int)speed*10; // the display interprets doubles werid they need to be multipled by 10 
          convert = (String) speed; // converts it to a sting for the concatonation
          Display.print("Speed.val =" + convert); // sends speed value 
          Display.write(0xff);
          Display.write(0xff);
          Display.write(0xff);

          dis_convert = (int) distance * 10;
          convert = (String) dis_convert;
          Display.print("Distance.val =" + convert); // sends the distance value
          Display.write(0xff);
          Display.write(0xff);
          Display.write(0xff);
        }
    }
      


    
}
