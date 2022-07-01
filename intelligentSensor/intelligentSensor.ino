/*
   Intelligent Sensor

   Yann BLANC

   Permit any sensor from a known list to be used by an user.
   The user choose the type of measurement the sensor needs to operate,
   the data is sent in JSON format through an Ethernet connection.

   Works on Wiznet W5100S-EVB-Pico

   
*/


#include "Command.h"
#include "ISnetwork.h"
#include "behavior.h"
#include "RPi_Pico_ISR_Timer.h"

ISnetwork network;
behavior behavior;

struct Command cmdA,cmdB,cmdC,cmdD,cmdE,cmdF;

void setup() {
  LittleFS.begin();
  pinMode(15, OUTPUT);
  pinMode(3, OUTPUT);
  digitalWrite(3,HIGH);
 
  //Check for previous configuration
  network.networkSetup();
  behavior.sensorSetup();
  delay(2000);
  
  behavior.checkPreviousConfiguration(&cmdA,&cmdB,&cmdC,&cmdD,&cmdE,&cmdF);
  
if(cmdA.mode>0) {
  Serial.println(cmdA.mode);
  behavior.behaviorHandler(cmdA, network.userQuery, network.client);
}
delay(30);
if(cmdB.mode>0) {
  Serial.println(cmdB.mode);
  behavior.behaviorHandler(cmdB, network.userQuery, network.client);
}
delay(30);
if(cmdC.mode>0) {
  Serial.println(cmdC.mode);
  behavior.behaviorHandler(cmdC, network.userQuery, network.client);
}
delay(30);
if(cmdD.mode>0) {
  Serial.println(cmdD.mode);
  behavior.behaviorHandler(cmdD, network.userQuery, network.client);
}
delay(30);
if(cmdE.mode>0) {
  Serial.println(cmdE.mode);
  behavior.behaviorHandler(cmdE, network.userQuery, network.client);
}
delay(30);
if(cmdF.mode>0) {
  Serial.println(cmdF.mode);
  behavior.behaviorHandler(cmdF, network.userQuery, network.client);
}

}

void loop() {


  while ( network.networkCheck() < 0) { }

  behavior.behaviorHandler(network.queryAK(), network.userQuery, network.client);

    
  if (network.userQuery.connected()) {
    network.endQuery();
  }
}
