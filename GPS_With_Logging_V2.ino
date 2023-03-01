#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// pin assignments
const int chipSelect = 53;
const int lapPin = 2;

Adafruit_MPU6050 mpu;
TinyGPSPlus GPS; // creates the gps object
SoftwareSerial Display (12,13); // sets up the software Serial to digital pins 12 and 13

// global variables used to store data

// for GPS data
double latitude;
double longitude;
double previousLatitude;
double previousLongitude;
int satellites;
double speed;
double altitude;
double heading;
double distance;
double currentDistanceBetween;

// for logging
String dataString;
String fileName = "";
String startHour;

//for timers
unsigned long startTime;
unsigned long totalTime;
unsigned long lapTime;
int totalMinutes;
int totalSeconds;
int lapMinutes;
int lapSeconds;
int lapCount = 0;
unsigned long lapPoint;
unsigned long prevTime;
bool nextLap = false;
bool first_cords = true; // used to set the prevlng and prevlat to the right starting value
bool flag = true; // used for the creation of the folder
const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
int minVal=265;
int maxVal=402;
double x;
double y;
double z;
/**
 * Default arduino function that runs once when the arduino is first powered/reset.
 * We use this function to start serial connections to the GPS and the Display
 */
void setup() {
  Serial2.begin(9600); // GPS port
  Serial1.begin(115200);
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
   if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
   }
  Serial.println("GPS connected");
  startTime = millis();
  lapPoint = startTime;
  pinMode(lapPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(lapPin),lapEvent, RISING);
mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }
 sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
}
void lapEvent(){
   nextLap = true; 
}

void Timers() {
  if (nextLap) {
    lapCount++; 
    lapPoint = millis();
    nextLap = false;      
  } 
  totalTime = millis() - startTime;
  lapTime = totalTime - lapPoint;

  totalMinutes = ((totalTime / 1000) / 60) % 60;
  totalSeconds = ((totalTime / 1000) % 60);

  setDisplayVar("TOTMIN", "txt", "\"" + normalizeTen(totalMinutes) + "\"");
  setDisplayVar("TOTSEC", "txt", "\"" + normalizeTen(totalSeconds) + "\"");

  lapMinutes = ((lapTime / 1000) / 60) % 60;
  lapSeconds = ((lapTime / 1000) % 60);

  setDisplayVar("LAPMIN", "txt", "\"" + normalizeTen(lapMinutes) + "\"");
  setDisplayVar("LAPSET", "txt", "\"" + normalizeTen(lapSeconds) + "\"");
}

void makeFolder() { // makes the folder using the month and date
  Serial.println(GPS.date.month()); 
  fileName.concat(String(GPS.date.month()));
  fileName.concat("_");
  fileName.concat(String(GPS.date.day()));
  SD.mkdir(fileName);
  fileName.concat("/");
  fileName.concat(GPS.time.hour());
      
}

String dateTime() {
  return String(GPS.time.hour()) + ":"+ String(GPS.time.minute())+ ":"+ String(GPS.time.second()); 
}

void setLog() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  dataString = normalizeTen(totalMinutes)+":"+ normalizeTen(totalSeconds) +","+normalizeTen(lapMinutes)+":"+
  normalizeTen(lapSeconds) +","+ String(lapCount)+","+ String(lng) +","+
  String(latitude) +","+ String(speed) +","+ String(distance) +","+ String(altitude) + ","+ String(a.acceleration.x)+ "," +String(a.acceleration.y)+","+
  String(a.acceleration.z)+ ","+ String(x)+ "," + String(y) + "," + String(z)+ "," + String(g.gyro.roll) + ","+
  g.gyro.pitch+ "," + g.gyro.heading);
}

void SD_loop() {
  if (flag) { // first run protocal
    makeFolder();
    Serial.println(fileName);
    if (SD.exists(fileName)==1) { // checks if a run has happened in that hour
      fileName.concat("_");
      fileName.concat(GPS.time.minute());           
    } 
    fileName.concat(".txt"); //saves as a txt file that can be imported into excell       
    flag = false;
    File logFile = SD.open(fileName, FILE_WRITE);
    logFile.println("totalTime,lapTime,Lap,Longitude,Latitude,Speed,Distance,Altitude"); // header of the file
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
  //check everything is good with the GPS before trying to run the main loop code
  //if any of these checks fail the loop returns before the rest of the code can be run
  if (Serial2.available() < 0) { // check if the GPS is available by checking if there is data in the buffer
    Serial.print("GPS unavailable\n");
    return;
  }

  GPS.encode(Serial2.read()); // sends the NHEMA sentence to be parsed

  if (not GPS.location.isValid()) { // checks if the GPS location is valid
    Serial.print("GPS Bad Location\n");
    return;
  }

  if (not GPS.location.isUpdated()) { // if the GPS data has been updated
    Serial.print("GPS Location not updated\n");
    return;
  }

  if (firstCoords) { // first time protocol
    firstCoords = false; // coverts it to false
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

  //TODO: COMMENT THIS
  currentDistanceBetween = GPS.distanceBetween(prevLatitude, prevLongitude,latitude, longitude);
  if (currentDistanceBetween > 3) { // if the distance is above around 9ft
    distance += currDistBetween / 1610.0; // converts the meters to miles
    setDisplayVar("Distance", "val", displayFormatted(distance));
    prevLongitude = longitude; // updates the prev values
    prevLatitude = latitude;
  }

  setDisplayVar("Speed", "val", displayFormatted(speed));
}
void angle_Loop(){ // loop for the angle and acceleration
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,19,true);
  AcX=Wire.read()<<8|Wire.read();
  AcY=Wire.read()<<8|Wire.read();
  AcZ=Wire.read()<<8|Wire.read();
  int xAng = map(AcX,minVal,maxVal,-90,90);
  int yAng = map(AcY,minVal,maxVal,-90,90);
  int zAng = map(AcZ,minVal,maxVal,-90,90);

  x= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
  y= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
  z= RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);
}
String displayFormatted(double input) {
  // the display interprets doubles weird they need to be multiplied by 10
  return (String) (int) (input * 10);
}

void setDisplayVar(String variable, String type, String newValue) {
  //send the updated value to the display
  Display.print(variable + "." + type + "=" + newValue);
  // tells the display to update info (not completely sure how...)
  Display.write(0xff);
  Display.write(0xff);
  Display.write(0xff);
}

String normalizeTen(int time) { // used for clock values just for looks
  return time < 10 ? ("0" + String(time)) : (String(time));
}

/**
 * Default arduino function that runs repeatedly while the arduino is on.
 */
void loop() {
  GPS_loop();

  angle_Loop();
  if (millis()-prevTime>1000) { // only writes to the SD every second
    SD_loop();
    Timers(); // timers function needs to be here because the millis() funtion uses interupts which messes with the way Serial communication works
    prevTime = millis();      
  } 
}
