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




//RPI_PICO_Timer ITimer0(0);
void setup() {
  pinMode(15, OUTPUT);
  network.networkSetup();
  behavior.sensorSetup();
  
  /* if (ITimer0.attachInterruptInterval(1000 * 1000, behavior.measure)) {
      Serial.println("Timer Start OK");

    } else {
      Serial.println("Failed to start timer");
    }*/
}

void loop() {


  while ( network.networkCheck() < 0) { }
//Serial.println("loop");
  behavior.behaviorHandler(network.queryAK(), network.userQuery);

    
  if (network.userQuery.connected()) {
    network.endQuery();
  }
}
