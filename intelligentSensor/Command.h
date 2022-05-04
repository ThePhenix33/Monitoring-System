#ifndef Command_H
#define Command_H

#include <Arduino.h>
#include "Wire.h"
#include "SHT31.h"
#include <ArduinoJson.h>
#include <Ethernet.h>



class Command {
private:

public:
int mode=-1;
int id=-1; 
int readingPeriod=-1; 
int min=-1;
int max=-1; 
int logicalLevel=-1;
int dataBank=-1; 
int interrupt=-1;

Command( int mode=-1,int id=-1, int readingPeriod=-1, int min=-1, int max=-1, int logicalLevel=-1,int dataBank=-1, int interrupt=-1){
  
  this->mode=mode;
  this->id=id;
  this->readingPeriod=readingPeriod;
  this->min=min;
  this->max=max;
  this->logicalLevel=logicalLevel;
  this->dataBank=dataBank;
  this->interrupt=interrupt;
};


};
#endif
