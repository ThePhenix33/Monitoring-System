#ifndef Measure_H
#define Measure_H

#include <Arduino.h>
#include "Wire.h"
#include "SHT31.h"
#include <ArduinoJson.h>
#include <Ethernet.h>

struct Measure{

float data=-1;
unsigned short time=-1;

};

#endif
