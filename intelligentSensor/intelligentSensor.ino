/*
 * Intelligent Sensor
 * 
 * Yann BLANC
 * 
 * Permit any sensor from a known list to be used by an user.
 * The user choose the type of measurement the sensor needs to operate,
 * the data is sent in JSON format through an Ethernet connection.
 * 
 * Works on Wiznet W5100S-EVB-Pico
 */

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SDAPIN  4
#define SCLPIN  5
#define RSTPIN  7
#define SERIAL SerialUSB
#else
#define SDAPIN  4
#define SCLPIN  5
#define RSTPIN  2
#define SERIAL Serial
#endif


#include "Command.h"
 #include "ISnetwork.h"
 #include "measure.h"
ISnetwork network;
measure measure;
String sensor[5], mode[5];
Command* testA = new Command(2,3,6,5,4,1,2,69);
Command* testB = new Command(5,6,8,1);
Command* testC = new Command();
void setup() {
  pinMode(15,OUTPUT);
 network.networkSetup();
 measure.sensorSetup();
}

void loop() {
  Serial.println(testA->mode);
  Serial.println(testB->interrupt);
    Serial.println(testB->mode);
      
      while( network.networkCheck()<0){
  }
  delay(100);
   

 measure.measurement(network.queryAK(),network.userQuery);
 
 measure.measurement(sensor[1],mode[1],network.userQuery);

if(sensor[0].equals("pir")&&mode[0].equals("1")){
  attachInterrupt(digitalPinToInterrupt(12),pirHandler,RISING);
}
if(network.userQuery.connected()){
  network.endQuery();
}
 sensor[0]=" ";
 mode[0] =" ";

   }

   void pirHandler(){
  Serial.println("PIR CAPTE");
}
