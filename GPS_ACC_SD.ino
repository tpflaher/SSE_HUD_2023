#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// pin assignments
const int chipSelect = 53; //TODO: description
/** The pin indicating if the lap button is being pressed */
const int lapPin = 2;

Adafruit_MPU6050 mpu; //TODO: description
/** Creates the GPS object */ //TODO: revisit this description
TinyGPSPlus GPS;
/** Defines the SoftwareSerial object Display, and binds it to the digital pins 12 and 13 */
SoftwareSerial Display (12,13);

// global variables used to store data

// for GPS data
double latitude;
double longitude;
double previousLatitude;
double previousLongitude;
int numberOfSatellites;
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
/** Used to set previousLatitude and previousLongitude to the right starting values */
bool firstCoords = true;
/** Used for the creation of the folder */
bool flag = true; //TODO: rename variable and rewrite its description
const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
int minVal=265;
int maxVal=402;
double x;
double y;
double z;

/**
 * Default arduino function that runs once when the arduino is first powered/reset.
 * We use this function to start serial connections to the GPS and the Display.
 * TODO: and a whole bunch of other stuff...
 */
void setup() { //TODO: break this up into functions for readability
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
  // while(GPS.location.lat() == 0){
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

/**
 * Called via interrupt when the lap button is pressed.
 * Sets the value of nextLap to true so that functionality can be handled on the next loop of the code.
 * @param[out] nextLap The boolean that is set to true when this function is called
 */
void lapEvent() {
   nextLap = true; 
}

/**
 * Sets the values of the timer components on the Display.
 */
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

  setDisplayVariable("TOTMIN", "txt", "\"" + normalizeTen(totalMinutes) + "\"");
  setDisplayVariable("TOTSEC", "txt", "\"" + normalizeTen(totalSeconds) + "\"");

  lapMinutes = ((lapTime / 1000) / 60) % 60;
  lapSeconds = ((lapTime / 1000) % 60);

  setDisplayVariable("LAPMIN", "txt", "\"" + normalizeTen(lapMinutes) + "\"");
  setDisplayVariable("LAPSET", "txt", "\"" + normalizeTen(lapSeconds) + "\"");
}

/**
 * TODO: is this accurate?
 * Makes a unique folder in the SD card storage named MONTH_DAY/HOUR.
 */
void makeFolder() { // makes the folder using the month and date
  Serial.println(GPS.date.month()); 
  fileName.concat(String(GPS.date.month()));
  fileName.concat("_");
  fileName.concat(String(GPS.date.day()));
  //TODO: I don't understand what is happening here
  SD.mkdir(fileName);
  fileName.concat("/");
  fileName.concat(GPS.time.hour());
      
}

/**
 * Returns the current time as a String formatted as HH:MM:SS.
 * @return The current time as a String formatted as HH:MM:SS
 */
String dateTime() { //TODO: rename this since it doesn't give the date?
  return String(GPS.time.hour()) + ":"+ String(GPS.time.minute())+ ":"+ String(GPS.time.second()); 
}

/**
 * Writes all relevant log data to the dataString variable in CSV format.
 * @param[out] dataString updated with a relevant log data in CSV format
 */
void setLog() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp); 
  dataString = normalizeTen(totalMinutes)+":"+ normalizeTen(totalSeconds) +","+normalizeTen(lapMinutes)+":"+
  normalizeTen(lapSeconds) +","+ String(lapCount)+","+ String(latitude) +","+
  String(longitude) +","+ String(speed) +","+ String(distance) +","+
  String(altitude) + ","+ String(a.acceleration.x)+ "," +String(a.acceleration.y)+","+
  String(a.acceleration.z)+ ","+ String(x)+ "," + String(y) + "," +
  String(z)+ "," + String(g.gyro.roll) + ","+ g.gyro.pitch + "," + g.gyro.heading;
}

/**
 * Makes a log file, and then writes data to it with each loop
 */
void SD_loop() {
  if (flag) { // first run protocol
    makeFolder();
    Serial.println(fileName);
    if (SD.exists(fileName)==1) { // checks if a run has happened in that hour
      fileName.concat("_");
      fileName.concat(GPS.time.minute());           
    } 
    fileName.concat(".txt"); //saves as a txt file that can be imported into excel
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

/**
 * Updates the Display with current information from the GPS
 */
void GPS_loop() {
  //check everything is good with the GPS before trying to run the main loop code
  //if any of these checks fail the loop returns before the rest of the code can be run

  //check if the GPS is available by checking if there is data in the buffer
  if (Serial2.available() < 0) {
    Serial.print("GPS unavailable\n");
    return;
  }

  //sends the NHEMA sentence to be parsed
  GPS.encode(Serial2.read());

  //checks if the GPS location is valid
  if (not GPS.location.isValid()) {
    Serial.print("GPS Bad Location\n");
    return;
  }

  //checks if the GPS data has been updated
  if (not GPS.location.isUpdated()) {
    Serial.print("GPS Location not updated\n");
    return;
  }

  //first time protocol
  if (firstCoords) {
    firstCoords = false; //sets this flag so that this code is skipped for the rest of the code's runtime
    // sets prevLatitude and prevLongitude to correct values
    previousLatitude = GPS.location.lat();
    previousLongitude = GPS.location.lng();
  }

  //set the GPS variables
  latitude = GPS.location.lat();
  longitude = GPS.location.lng();
  altitude = GPS.altitude.feet(); //the GPS altitude in feet
  speed = GPS.speed.mph();
  numberOfSatellites = GPS.satellites.value();
  heading = GPS.course.deg();

  //TODO: COMMENT THIS
  currentDistanceBetween = GPS.distanceBetween(previousLatitude, previousLongitude,latitude, longitude);
  if (currentDistanceBetween > 3) { // if the distance is above around 9ft
    distance += currentDistanceBetween / 1610.0; // converts the meters to miles
    setDisplayVariable("Distance", "val", displayFormatted(distance));
    previousLongitude = longitude; // updates the prev values
    previousLatitude = latitude;
  }

  setDisplayVariable("Speed", "val", displayFormatted(speed));
}

/**
 * Updates the angle and acceleration TODO: where is acceleration updated?
 * @param[out] x the x component of angular acceleration?
 * @param[out] y the y component of angular acceleration?
 * @param[out] z the z component of angular acceleration?
 */
void angleLoop() { // loop for the angle and acceleration
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,19,true);
  AcX=Wire.read()<<8|Wire.read();
  AcY=Wire.read()<<8|Wire.read();
  AcZ=Wire.read()<<8|Wire.read();
  int xAng = map(AcX,minVal,maxVal,-90,90);
  int yAng = map(AcY,minVal,maxVal,-90,90);=
  int zAng = map(AcZ,minVal,maxVal,-90,90);
 
  x = RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
  y = RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
  z = RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);
}

/** TODO: CHANGE "INTERPRETS DOUBLES WEIRD" TO AN ACTUAL EXPLANATION
 * Because the display interprets doubles weird, they need to be multiplied by 10
 * and then sent as a string representation of an integer to be correctly rendered.
 * @param input the value to be formatted
 * @return a String representation of the input double that renders correctly on the Heads Up Display
 */
String displayFormatted(double input) {
  // the display interprets doubles weird they need to be multiplied by 10
  return (String) (int) (input * 10);
}

/**
 * Sets a variable on the Heads Up Display to a given value.
 * @param variable the String name of the variable (set in the Nextion software) to set the value of
 * @param type the data type of the variable being updated, "val" for numbers, "txt" for text
 * @param newValue the new value to set the variable to
 */
void setDisplayVariable(String variable, String type, String newValue) {
  //send the updated value to the display
  Display.print(variable + "." + type + "=" + newValue);
  //tells the display to update info (not completely sure how...)
  Display.write(0xff);
  Display.write(0xff);
  Display.write(0xff);
}

/**
 * Adds a leading zero to the input number if it is less than 10.
 * Used to make the clock values look better.
 * @param time the time to potentially add a leading zero to
 * @return a String representation of the number, including leading zero if needed
 */
String normalizeTen(int time) {
  return time < 10 ? ("0" + String(time)) : (String(time));
}

/**
 * Default arduino function that runs repeatedly while the arduino is on.
 */
void loop() {
  GPS_loop();

  angleLoop();
  if (millis()-prevTime>1000) { // only writes to the SD every second
    SD_loop();
    // timers function needs to be here because the millis() function uses interrupts which messes with the way Serial communication works
    Timers();
    prevTime = millis();      
  } 
}
