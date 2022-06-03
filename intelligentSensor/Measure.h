/*
   Command.h

   Yann BLANC

   struct that permits easy recover of the parameters sent by
   the server via a HTTP request.
   
*/

#ifndef Measure_H
#define Measure_H

#include <Arduino.h>
#include "Wire.h"
#include "SHT31.h"
#include <ArduinoJson.h>
#include <Ethernet.h>

struct Measure{

float data=-1;
unsigned long time=-1;

};

#endif
