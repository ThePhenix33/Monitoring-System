#ifndef ISnetwork_H
#define ISnetwork_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Ethernet.h>
#include "Command.h"

class ISnetwork {
private:

public:
EthernetClient userQuery;
int networkCheck();
void networkSetup();
Command queryAK(String*,String*);
void endQuery();
};
#endif
