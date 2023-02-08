// grabs and prints the data of the ecu intake temp once a second.
byte buff[32];
double intake_Temp;
long prevtime;
void setup() {
  // put your setup code here, to run once:
  Serial3.begin(115200);
  Serial.begin(9600);
  prevtime = millis();

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial3.readBytes(buff,32);
  intake_Temp = (buff[14] *256 + buff[15])*1.25- 40;
  if(millis() - prevtime > 1000){
    Serial.println(intake_Temp); 
  }
}
