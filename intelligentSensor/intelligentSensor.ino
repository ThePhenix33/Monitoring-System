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


void setup() {
  
  pinMode(15, OUTPUT);
  network.networkSetup();
  behavior.sensorSetup();
 
}

void loop() {


  while ( network.networkCheck() < 0) { }

  behavior.behaviorHandler(network.queryAK(), network.userQuery, network.client);

    
  if (network.userQuery.connected()) {
    network.endQuery();
  }
}
