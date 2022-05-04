#ifndef measure_H
#define measure_H

#include <Arduino.h>
#include "Wire.h"
#include "SHT31.h"
#include <ArduinoJson.h>
#include <Ethernet.h>
#include "Command.h"


class measure {
private:
int sensorScan();
public:
void sensorSetup();
static void measurement(struct Command, EthernetClient);

};
#endif
