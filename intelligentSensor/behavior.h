#ifndef behavior_H
#define behavior_H

#include <Arduino.h>
#include "Wire.h"
#include "SHT31.h"
#include <ArduinoJson.h>
#include <Ethernet.h>
#include "Command.h"
#include "RPi_Pico_TimerInterrupt.h"


class behavior  {
  
private:
int nbDevices;
int connectedSensors[128];
void sensorScan();

struct Command activeBehavior, lastBehavior, activeCommand;
EthernetClient activeQuery;

RPI_PICO_Timer ITimer0(0);
RPI_PICO_Timer ITimer1(1);
RPI_PICO_Timer ITimer2(2);
RPI_PICO_Timer ITimer3(3);

public:

void sensorSetup();
 void behaviorHandler(struct Command activeCommand, EthernetClient activeQuery);
void ISinfo();
void regularMeasure();
void regularMeasureAlert();
void regularMeasureAlertDataLess();
void detection();
void unitaryMeasure();
void dataBankRead();
void behaviourStop();

};
#endif
