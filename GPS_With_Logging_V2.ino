#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <SD.h>
#include <SPI.h>

const int chipSelect = 53;

TinyGPSPlus GPS; // creates the gps object
SoftwareSerial Display (12,13); // sets up the software Serial to digital pins 12 and 13

// global variables used to store data
double lat;
double lng;
double prevlat;
double prevlng;
int sats;
double speed;
double alt;
double heading;
double distance;
String dataString;
String fileName = "";
String startHour;
unsigned long Start_Time;
unsigned long TOTTIME;
int TOTMIN;
int TOTSEC;

unsigned long prevTime;

bool first_cords = true; // used to set the prevlng and prevlat to the right starting value
bool flag = true; // used for the creation of the folder 
/**
 * Default arduino function that runs once when the arduino is first powered/reset.
 * We use this function to start serial connections to the GPS and the Display
 */
void setup() {
  Serial2.begin(9600); // GPS port
  Display.begin(9600); // Display software serial
  Serial.begin(9600); 
  prevTime = millis();   
  startHour = String(GPS.time.hour());
  Serial.println(startHour);
  while (!Serial) {
   // wait for serial port to connect. Needed for native USB port only
  } 
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
 // while(GPS.location.lat() == 0){ //
   // Serial.println("Waiting for GPS to connect");  
    //delay(1000);  
  //}
Serial.println("GPS connected");
  
}
void Timers(){  
  TOTTIME = millis();  
}
void makeFolder(){ // makes the folder using the month and date
  Serial.println(GPS.date.month()); 
  fileName.concat(String(GPS.date.month()));
  fileName.concat("_");
  fileName.concat(String(GPS.date.day()));
  SD.mkdir(fileName);
  fileName.concat("/");
  fileName.concat(GPS.time.hour());
      
}
String dateTime(){
  return String(GPS.time.hour()) + ":"+ String(GPS.time.minute())+ ":"+ String(GPS.time.second()); 
}

void setLog(){
  dataString = dateTime() +","+ String(lng) +","+ String(lat) +","+ String(speed) +","+ String(distance) +","+ String(alt);  
}
void SD_loop(){    
  if(flag){ // first run protocal
    makeFolder();
    Serial.println(fileName);
    if(SD.exists(fileName)==1){ // checks if a run has happened in that hour 
      fileName.concat("_");
      fileName.concat(GPS.time.minute());           
    } 
    fileName.concat(".txt"); //saves as a txt file that can be imported into excell       
    flag = false;
    File logFile = SD.open(fileName, FILE_WRITE);
    logFile.println("Time,Logitude,Latitude,Speed,Distance,Altitude"); // header of the file 
    logFile.close();      
  }
  File logFile = SD.open(fileName, FILE_WRITE); // creates the file 

  // if the file is available, write to it:
  if (logFile) { 
    setLog();    
    logFile.println(dataString);
    logFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening the file");
  }  
}
void GPS_loop() {
  // put your main code here, to run repeatedly:
    if(Serial2.available()>0){ // if the GPS data is availibe
      GPS.encode(Serial2.read()); // sends the NHEMA setance to be parse
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

          if(GPS.distanceBetween(prevlat,prevlng,lat,lng)>3) { // if the distance is above around 9ft
             distance += (GPS.distanceBetween(prevlat,prevlng,lat,lng)/1610.0); // converts the meters to miles
              prevlng = lng; // updates the prev values
              prevlat = lat;
              
          setDisplayVar("Speed", displayFormatted(speed));
          //speed = (int)(speed*10); // the display interprets doubles werid they need to be multipled by 10
          //convert = (String) (int)speed; // converts it to a sting for the concatonation
          //Display.print("Speed.val=" + convert); // sends speed value


          setDisplayVar("Distance", displayFormatted(distance));
          //dis_convert = (int) (distance * 10); // converts the value to an int
          //convert = (String) dis_convert;
          //Display.print("Distance.val=" + convert); // sends the distance value
        }        
      }
    }
  }
}
String displayFormatted(double input) {
  // the display interprets doubles werid they need to be multipled by 10
  return (String) (int) (input * 10);
}
void setDisplayVar(String variable, String newValue) { 
  Display.print(variable + ".val=" + newValue);
  updateInfo();
}
void updateInfo() {
  Display.write(0xff); // tells the display to update info
  Display.write(0xff);
  Display.write(0xff);
}
String normalizeTen(int time) { // used for clock values just for looks
    return time < 10 ? ("0" + String(time)) : (String(time));
}
void loop(){ 
  GPS_loop();
  if(millis()-prevTime>1000){ // only writes to the SD every second
      SD_loop();
    prevTime = millis();      
  }   
}