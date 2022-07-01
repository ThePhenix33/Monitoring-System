/*
   Command.h

   Yann BLANC

   struct that permits easy recover of the parameters sent by
   the server via a HTTP request.
   
*/
#ifndef Command_H
#define Command_H

#include <Arduino.h>
#include "Wire.h"
#include "SHT31.h"
#include <ArduinoJson.h>
#include <Ethernet.h>

struct Command{

int mode=-1;
int id=-1; 
int readingPeriod=-1; 
float min=-1;
float max=-1; 
int logicalLevel=-1;
int databank=-1; 
int timer=-1;
int timerStart=-2;
int interrupt=-1;
int flagReset=0;
int flagMin = 0;
int flagMax = 0;
};

#endif
