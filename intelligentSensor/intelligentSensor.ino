/*
   Intelligent Sensor

   Yann BLANC

   Permit any sensor from a known list to be used by an user.
   The user choose the type of measurement the sensor needs to operate,
   the data is sent in JSON format through an Ethernet connection.

   Works on Wiznet W5100S-EVB-Pico
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
#include "behavior.h"
ISnetwork network;
behavior behavior;
String sensor[5], mode[5];

void setup() {
  pinMode(15, OUTPUT);
  network.networkSetup();
  behavior.sensorSetup();
}

void loop() {


  while ( network.networkCheck() < 0) {
  }
  delay(100);


  behavior.behaviorHandler(network.queryAK(), network.userQuery);

  if (sensor[0].equals("pir") && mode[0].equals("1")) {
    attachInterrupt(digitalPinToInterrupt(12), pirHandler, RISING);
  }
  if (network.userQuery.connected()) {
    network.endQuery();
  }
  sensor[0] = " ";
  mode[0] = " ";

}

void pirHandler() {
  Serial.println("PIR CAPTE");
}
