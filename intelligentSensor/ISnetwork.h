/*
   ISnetwork

   Yann BLANC

   Handle the Intelligent Sensor network aspect,
   from connecting to the global network to handle
   HTTP requests.
  
*/

#ifndef ISnetwork_H
#define ISnetwork_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Ethernet.h>
#include "Command.h"

class ISnetwork {
private:

public: 

EthernetClient userQuery,client;
int networkCheck();
void networkSetup();

struct Command queryAK();
void endQuery();
};
#endif
