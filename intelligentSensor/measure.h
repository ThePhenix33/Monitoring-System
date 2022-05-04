#ifndef measure_H
#define measure_H

#include <Arduino.h>
#include "Wire.h"
#include "SHT31.h"
#include <ArduinoJson.h>
#include <Ethernet.h>



class measure {
private:
int sensorScan();
public:
void sensorSetup();
static void measurement(String, String, EthernetClient);

};
#endif
