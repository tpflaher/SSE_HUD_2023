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
const int chipSelect = 53;
const int lapPin = 2;

Adafruit_MPU6050 mpu;
TinyGPSPlus GPS; // creates the gps object
SoftwareSerial Display (12,13); // sets up the software Serial to digital pins 12 and 13
// global variables used to store data
float hourConversion;
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
unsigned long LAPTIME;
int TOTMIN;
int totalAverageCount=0;
int lapAverageCount = 0;
double averageSpeed;
double lapAverageSpeed;
double lapAverageSpeedNeeded;
int MAXSECS = 2040;
int LAPGOAL = 510;
int TOTSEC;
int LAPMIN;
int LAPSEC;
int lapCount =-1;
unsigned long lapPoint;
unsigned long prevTime;
unsigned long ECUTime;
bool speedWatch = false; // used to see if speed has dropped below the optimal zone
bool speedCritical = false;// ^
bool nextLap = false;
bool first_cords = true; // used to set the prevlng and prevlat to the right starting value
bool folderFirst = true; // used for the creation of the folder 
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
 // while(GPS.location.lat() == 0){ //
   // Serial.println("Waiting for GPS to connect");  
    //delay(1000);  
  //}
   if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
   }  
  Serial.println("GPS connected");
  Start_Time = millis();
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
void lapEvent(){
   nextLap = true; 
}
void Timers(){  
  if(nextLap){
        lapCount++;
       lapPoint = millis();
 

    if(lapCount==1){
      Start_Time = millis();
      LAPTIME =0;          
    }    
    if(lapCount != 0){
      MAXSECS -= LAPTIME / 1000;
      LAPGOAL = MAXSECS/(4-(lapCount-1));
      hourConversion = (LAPGOAL/60.0)/60.0; 
      lapAverageSpeedNeeded = 2.4 / hourConversion;
      setDisplayVar("SPDND", displayFormatted(lapAverageSpeedNeeded));      
      //Start_Time = millis();
    }      
      int GOALMIN = ((LAPGOAL/60) % 60);
      int GOALSEC = (LAPGOAL % 60); 
    nextLap = false; 
    Display.print("GOALMIN.txt=\"" +normalizeTen(GOALMIN)+"\""); 
    updateInfo();
    Display.print("GOALSEC.txt=\"" +normalizeTen(GOALSEC)+"\""); 
    updateInfo();  
  } 
  lapAverageCount++;
  totalAverageCount++;
  if(lapAverageCount>1){
    lapAverageSpeed = (lapAverageSpeed*(lapAverageCount -1)+speed)/lapAverageCount;
    setDisplayVar("LAPAVG", displayFormatted(lapAverageSpeed));
  }
  if(totalAverageCount>1){
    averageSpeed = (averageSpeed*(totalAverageCount -1)+speed)/totalAverageCount;
    setDisplayVar("AVGSPD", displayFormatted(averageSpeed));
  }
      
  TOTTIME = millis() - Start_Time;
  LAPTIME = millis() -lapPoint;    
  Display.print("TOTMIN.txt=\"" +normalizeTen(((TOTTIME / 1000)/60) % 60)+"\""); 
  updateInfo();
  TOTMIN = ((TOTTIME / 1000)/60) % 60;
  Display.print("TOTSEC.txt=\"" +normalizeTen((TOTTIME/1000) % 60)+ "\""); // sends the distance value
  updateInfo();
  TOTSEC = ((TOTTIME/1000) % 60);
  LAPMIN = ((LAPTIME / 1000)/60) % 60;
  Display.print("LAPMIN.txt=\"" +normalizeTen(LAPMIN)+"\""); 
  updateInfo();
  LAPSEC = ((LAPTIME/1000) % 60);
  Display.print("LAPSEC.txt=\"" +normalizeTen(LAPSEC)+ "\""); // sends the distance value
  updateInfo();
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
   sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp); 
  dataString = (normalizeTen(TOTMIN)+":"+ normalizeTen(TOTSEC) +","+normalizeTen(LAPMIN)+":"+
   normalizeTen(LAPSEC) +","+ String(lapCount+1)+","+ String(lng) +","+
   String(lat) +","+ String(speed) +","+ String(distance) +","+ String(alt) + ","+ String(a.acceleration.x)+ "," +String(a.acceleration.y)+","+
   String(a.acceleration.z)+ ","+ String(x)+ "," + String(y) + "," + String(z)+ "," + String(g.gyro.roll) + ","+
   g.gyro.pitch+ "," + g.gyro.heading +","+String(temp.temperature)+","+ String(RPM) +","+ String(MAP) +","+ String(TPS) +","+ String(ECT) + ","+ String(IAT)+ "," +String(O2S)+","+
    String(SPARK) +","+ String(FUELPW1) +","+ String(FUELPW2) +","+ String(UbAdc) + ","+ String(BARO)+ "," +String(FuelComsumption));  
}
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
    logFile.println("TOTTIME,LAPTIME,Lap,Logitude,Latitude,Speed,Distance,Altitude,AcX,AcY,AcZ,TiltX,TiltY,TiltZ,GryoX,GyroY,GyroZ,Temp,RPM,MAP,TPS,ECT,IAT,O2S,SPARK,FUELPW1,FUELPW2,UbAdc,FUELLvl,BARO,Fuel_Consumption"); // header of the file 
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
      GPS.encode(Serial2.read()); // sends the NHEMA setance to be parsed
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
          }
          
          if(speed>=29 || speed <= 5){// if within 5 mph of the borders for the run then the speed will change to a yellow colour
              if(!speedCritical){
                Display.print("SPEED.pco=63488"); 
                updateInfo();
                speedWatch = false;
                speedCritical = true;
              }
            
          }else if(speed <= 10||speed >= 25){ // if less than or equal to the end points then the speed will change to a red colour
              if(!speedWatch){
                Display.print("SPEED.pco=65504");
                updateInfo();
                speedWatch = true;
                speedCritical = false;                
              }
          }else{ // if neither of the conditions above are met check to see if the speed should be changed back to the default green colour
              if(speedWatch){
                Display.print("SPEED.pco=1024");
                updateInfo();
                speedWatch = false;                              
              }
          }
                                       
            setDisplayVar("SPEED", displayFormatted(speed));
          //speed = (int)(speed*10); // the display interprets doubles werid they need to be multipled by 10
          //convert = (String) (int)speed; // converts it to a sting for the concatonation
          //Display.print("Speed.val=" + convert); // sends speed value


          setDisplayVar("DISTANCE", displayFormatted(distance));
          //dis_convert = (int) (distance * 10); // converts the value to an int
          //convert = (String) dis_convert;
          //Display.print("Distance.val=" + convert); // sends the distance value
        }        
      }
    }
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
   bool ECU(){
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
  angle_Loop();
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

