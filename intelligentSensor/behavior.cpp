#include "behavior.h"



#define SHT35_ADDRESS   0x45
SHT31 sht;




//EthernetClient activeQuery;
//struct Command activeBehavior,lastBehavior, activeCommand;

void behavior::sensorScan() {

  byte error, address;


  Serial.println("Scanning...");

  nbDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      connectedSensors[nbDevices] = address;
      nbDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nbDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");


}





void behavior::sensorSetup() {
  Wire.begin();
  sensorScan();
  for (int i  = 0; i <= nbDevices; i++) {
    Serial.println(nbDevices);
    switch (connectedSensors[i]) {

      case 0x45: //SHT35

        sht.begin(SHT35_ADDRESS);
        Wire.setClock(100000);
        sht.requestData();
        break;

    }
  }

}

/*MODE 0 : INTELLIGENT SENSOR INFO

   Returns what are the connected sensors,
   what are the active meausre modes ,
   and the last mode selected with the
   associated parameters.

*/
void behavior::ISinfo() {
  Serial.println("INTELLIGENT SENSOR INFO");
  DynamicJsonDocument doc(1024);

  for (int i = 0; i < nbDevices; i++) {
    doc["sensors"][i] = connectedSensors[i];
  }

  doc["activeBehavior"]["mode"] = activeBehavior.mode;
  doc["activeBehavior"]["sensor"] = activeBehavior.id;
  doc["activeBehavior"]["interrupt"] = activeBehavior.interrupt;

  doc["lastBehavior"]["mode"] = lastBehavior.mode;
  doc["lastBehavior"]["sensor"] = lastBehavior.id;
  doc["lastBehavior"]["interrupt"] = lastBehavior.interrupt;

  serializeJson(doc, activeQuery);

  activeQuery.stop();
  Serial.println("End of query after JSON sending \n\n");
}



/*MODE 9 : UNITARY MEASURE

   Returns selected sensor entry data

*/
void behavior::unitaryMeasure() {

  Serial.println("UNITARY MEASURE");
  lastBehavior = activeCommand;
  switch (activeCommand.id) {
    case 1:
      {

        Serial.println(" >Temperature measurement..");

        DynamicJsonDocument doc(1024);

        doc["sensor"] = "sht35";

        doc["time"]   = millis();
        doc["measure"] = "temperature";


        if (sht.dataReady())
        {
          bool success  = sht.readData();
          sht.requestData();

          if (success == false)
          {
            Serial.println(" >Failed read");
          }
          else
          {
            doc["data"][0] = sht.getRawTemperature() * (175.0 / 65535) - 45;
          }
        }
        Serial.println(" >Data sent!");
        serializeJson(doc, activeQuery);


        activeQuery.stop();
        Serial.println("End of query after JSON sending \n\n");
        break;
      }
    case 2:
      {
        Serial.println(" >Humidity measurement..");

        DynamicJsonDocument doc(1024);

        doc["sensor"] = "sht35";
        doc["time"]   = millis();
        doc["measure"] = "humidity";



        if (sht.dataReady())
        {
          bool success  = sht.readData();
          sht.requestData();

          if (success == false)
          {
            Serial.println(" >Failed read");
          }
          else
          {
            doc["data"][0] = sht.getRawHumidity() * (100.0 / 65535);
          }
        }
        Serial.println(" >Data sent!");
        serializeJson(doc, activeQuery);


        activeQuery.stop();
        Serial.println("End of query after JSON sending \n\n");
        break;
      }
    case 5:
      {
        Serial.println(" >ADC measurement..");

        DynamicJsonDocument doc(1024);
        analogReadResolution(12);
        doc["sensor"] = "ADC";
        doc["time"]   = millis();
        doc["measure"] = "voltage";
        doc["data"] = analogRead(27);
        Serial.println(" >Data sent!");
        serializeJson(doc, activeQuery);

        activeQuery.stop();
        Serial.println("End of query after JSON sending \n\n");
        break;
      }
  }
}


void behavior::behaviorHandler(struct Command command, EthernetClient query) {

  this->activeCommand = command;
  this->activeQuery = query;

  switch (activeCommand.mode) {

    case 0:
      { Serial.print(">Selected mode: ");
        ISinfo();
        break;
      }
    case 9:
      { Serial.print(">Selected mode: ");
        unitaryMeasure();
        break;
      }
  }

}


void 
