#ifndef sensor_H
#define sensor_H

#include <Arduino.h>
#include "Command.h"
#include "Wire.h"
#include "SHT31.h"
#include <ArduinoJson.h>
#include <Ethernet.h>



class Sensor {
private:

public:
Sensor(int id);
command(Command command);

};
#endif
