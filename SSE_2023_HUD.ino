#include<Wire.h>
#include <TinyGPS++.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>  // Include the Servo library

int ECUData[32];
byte RS232[64];
int StartIndex;
bool RSchecker = true;
Servo throttle;   // Create a Servo object for the throttle
int trigger = A0; //throttle trigger
int throttleValue;          // Variable to store the potentiometer value
int prevThrottle = 0;
// 63488 is ID for red in nextion
// 65504 is the ID for yellow in nextion

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
float hourConversion;
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
int totalAverageCount=0;
int lapAverageCount = 0;
double averageSpeed;
double lapAverageSpeed;
double lapAverageSpeedNeeded;
int MAXSECS = 2040;
int LAPGOAL = 510;
int totalSeconds;
int lapMinutes;
int lapSeconds;
int lapCount = -1;
unsigned long lapPoint;
unsigned long prevTime;
unsigned long ECUTime;
bool speedWatch = false; // used to see if speed has dropped below the optimal zone
bool speedCritical = false;// ^
bool nextLap = false;
/** Used to set previousLatitude and previousLongitude to the right starting values */
bool firstCoords = true;
/** Used for the creation of the folder */
bool folderFirst = true;
const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
int minVal=265;
int maxVal=402;
double x;
double y;
double z;

double RPM;
double MAP;
double TPS;
double ECT;
double IAT;
double O2S;
double SPARK;
double FUELPW1;
double FUELPW2;
double UbAdc;
double FuelLvl;
double BARO;
double FuelComsumption;

/**
 * Default arduino function that runs once when the arduino is first powered/reset.
 * We use this function to start serial connections to the GPS and the Display
 */
void setup() {
  Serial2.begin(9600); // GPS port
  Serial1.begin(115200); // accelerometer
  Serial3.begin(115200);// ECU
  Display.begin(9600); // Display software serial
  Serial.begin(9600); //debug Port
  prevTime = millis();
  throttle.attach(9);
  pinMode(A0, INPUT);  
  startHour = String(GPS.time.hour());
  Serial.println(startHour);
  while (!Serial) {
   // wait for serial port to connect. Needed for native USB port only
  } 
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more
  }
  Serial.println("card initialized.");
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
  }  
  Serial.println("GPS connected");
  startTime = millis();
  lapPoint = millis();  
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
void Timers(){  
  if (nextLap) {
    lapCount++;
    lapPoint = millis();
 
    if(lapCount==1){
      startTime = millis();
      lapTime =0;          
    }

    if(lapCount != 0){
      MAXSECS -= lapTime / 1000;
      LAPGOAL = MAXSECS/(4-(lapCount-1));
      hourConversion = (LAPGOAL/60.0)/60.0; 
      lapAverageSpeedNeeded = 2.4 / hourConversion;
      //TOOD: explanation of what SPDND is
      setDisplayVariable("SPDND", "val", displayFormatted(lapAverageSpeedNeeded));      
    }

    int GOALMIN = ((LAPGOAL/60) % 60);
    int GOALSEC = (LAPGOAL % 60); 
    nextLap = false;
    setDisplayVariable("GOALMIN", "txt", "\"" + normalizeTen(GOALMIN) + "\"");
    setDisplayVariable("GOALSEC", "txt", "\"" + normalizeTen(GOALSEC) + "\"");
  } 

  lapAverageCount++;
  totalAverageCount++;
  if (lapAverageCount>1) {
    lapAverageSpeed = (lapAverageSpeed*(lapAverageCount -1)+speed)/lapAverageCount;
    setDisplayVariable("LAPAVG", "val", displayFormatted(lapAverageSpeed));
  }
  if (totalAverageCount>1) {
    averageSpeed = (averageSpeed*(totalAverageCount -1)+speed)/totalAverageCount;
    setDisplayVariable("AVGSPD", "val", displayFormatted(averageSpeed));
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
void makeFolder(){ // makes the folder using the month and date
  Serial.println(GPS.date.month()); 
  fileName.concat(String(GPS.date.month()));
  fileName.concat("_");
  fileName.concat(String(GPS.date.day()));
  SD.mkdir(fileName);
  //TODO: I don't understand what is happening here
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
void setLog(){
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp); 
  dataString = (normalizeTen(totalMinutes)+":"+ normalizeTen(totalSeconds) +","+normalizeTen(lapMinutes)+":"+
  normalizeTen(lapSeconds) +","+ String(lapCount)+","+ String(latitude) +","+
  String(longitude) +","+ String(speed) +","+ String(distance) +","+
  String(altitude) + ","+ String(a.acceleration.x)+ "," +String(a.acceleration.y)+","+
  String(a.acceleration.z)+ ","+ String(x)+ "," + String(y) + "," +
  String(z)+ "," + String(g.gyro.roll) + ","+ g.gyro.pitch + "," +
  g.gyro.heading +","+String(temp.temperature)+","+ String(RPM) +","+
  String(MAP) +","+ String(TPS) +","+ String(ECT) + ","+ String(IAT)+ ","+
  String(O2S)+","+String(SPARK) +","+ String(FUELPW1) +","+ String(FUELPW2) +","+
  String(UbAdc) + ","+ String(BARO)+ "," +String(FuelComsumption));  
}

/**
 * Makes a log file, and then writes data to it with each loop
 */
void SD_loop(){    
  if(folderFirst){ // first run protocal
    makeFolder();
    Serial.println(fileName);
    if(SD.exists(fileName)==1){ // checks if a run has happened in that hour 
      fileName.concat("_");
      fileName.concat(GPS.time.minute());           
    } 
    fileName.concat(".txt"); //saves as a txt file that can be imported into excell       
    folderFirst = false;
    File logFile = SD.open(fileName, FILE_WRITE);
    logFile.println("TotalTime,LapTime,Lap,Logitude,Latitude,Speed,Distance,Altitude,AcX,AcY,AcZ,TiltX,TiltY,TiltZ,GryoX,GyroY,GyroZ,Temp,RPM,MAP,TPS,ECT,IAT,O2S,SPARK,FUELPW1,FUELPW2,UbAdc,FUELLvl,BARO,Fuel_Consumption"); // header of the file 
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
    setDisplayVariable("DISTANCE", "val", displayFormatted(distance));
    previousLongitude = longitude; // updates the prev values
    previousLatitude = latitude;
  }
  
  if (speed>=29 || speed <= 5) {// if within 5 mph of the borders for the run then the speed will change to a yellow colour
    if(!speedCritical){
      setDisplayVariable("SPEED", "pco", "63488");
      speedWatch = false;
      speedCritical = true;
    }
  }
  else if (speed <= 10||speed >= 25) { // if less than or equal to the end points then the speed will change to a red colour
    if(!speedWatch){
      setDisplayVariable("SPEED", "pco", "65504");
      speedWatch = true;
      speedCritical = false;                
    }
  }
  else{ // if neither of the conditions above are met check to see if the speed should be changed back to the default green colour
    if(speedWatch){
      setDisplayVariable("SPEED", "pco", "1024");
      speedWatch = false;                              
    }
  }
                                
  setDisplayVariable("SPEED", "val", displayFormatted(speed));
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
  int yAng = map(AcY,minVal,maxVal,-90,90);
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
 * @param type the data type of the variable being updated, "val" for numbers, "txt" for text, "pco" for color
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

String normalizeTen(int time) { // used for clock values just for looks
    return time < 10 ? ("0" + String(time)) : (String(time));
}

bool ECU() {
	bool check = false;
	if(Serial3.available() > 0)
	{
		byte RS232 = 0;
		
		// Look for Ecotron's header, if not found return from function
		while(RS232 != 0x80){
			RS232 = Serial3.read();
			if(RS232 < 0) // Buffer is cleared and no header was found
			{
				return false;
			}
		}

    //might overflow 256?
		byte checksum = 0x80 + 0x8f;
		RS232 = Serial3.read();
		if(RS232 == 0x8f){
			for(int x = 2; x < 32 ; x++){
				ECUData[x] = (int)Serial.read();
				if(x == 31 && checksum == ECUData[x]) // When on last bit (checksum)
				{
					check = true;
				}
				checksum += ECUData[x];
			}
			RPM = ((ECUData[6]*256) + ECUData[7]) * 0.25;
			MAP = ((ECUData[8]*256) + ECUData[9]) * 0.0039;
			TPS = ((ECUData[10]*256) + ECUData[11]) * 0.0015;
			ECT = ((ECUData[12]*256) + ECUData[13]) * 1.25 - 40;
			IAT = ((ECUData[14]*256) + ECUData[15]) * 1.25 - 40;
			O2S = ((ECUData[16]*256) + ECUData[17]) * 0.0012;
			SPARK = ((ECUData[18]*256) + ECUData[19]) * 0.75;
			FUELPW1 = ((ECUData[20]*256) + ECUData[21]) * 0.001;
			FUELPW2 = ((ECUData[22]*256) + ECUData[23]) * 0.001;
			UbAdc = ((ECUData[24]*256) + ECUData[25]) * 0.4;
			FuelLvl = ((ECUData[26]*256)) * 0.0039;
			BARO = ((ECUData[27]*256) + ECUData[28]) * 0.0039;
			FuelComsumption = ((ECUData[29]*256) + ECUData[30]) * 0.0116;
		}
	}
	return check;
}     

void loop(){
  GPS_loop();
  angleLoop();
  if(millis()-prevTime>1000){ // only writes to the SD every second
      SD_loop();       
      Timers(); // timers function needs to be here because the millis() funtion uses interupts which messes with the way Serial communication works    
    prevTime = millis();      
  } 
  if(millis()-ECUTime>100){
    ECU();
      
    ECUTime = millis();      
  } 
}
