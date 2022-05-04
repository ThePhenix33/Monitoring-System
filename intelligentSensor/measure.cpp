#include "measure.h"



#define SHT35_ADDRESS   0x45
SHT31 sht;




int measure::sensorScan() {

  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
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
      return address;
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");


  return 0 ;
}





void measure::sensorSetup() {
  Wire.begin();

  switch (sensorScan()) {

    case 0x45: //SHT35

      sht.begin(SHT35_ADDRESS);
      Wire.setClock(100000);
      sht.requestData();
      break;

  }

}


void measure::measurement(String sens, String mode, EthernetClient query) {



  if (sens.equals("temp")) {

    Serial.println("Temperature measurement..");

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
        Serial.println("Failed read");
      }
      else
      {
        doc["data"][0] = sht.getRawTemperature() * (175.0 / 65535) - 45;
      }
    }
    Serial.println("Data sent!");
    serializeJson(doc, query);


    query.stop();
    Serial.println("End of query after JSON sending \n\n");
  }

  if (sens.equals("hum")) {

    Serial.println("Humidity measurement..");

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
        Serial.println("Failed read");
      }
      else
      {
        doc["data"][0] = sht.getRawHumidity() * (100.0 / 65535);
      }
    }
    Serial.println("Data sent!");
    serializeJson(doc, query);


    query.stop();
    Serial.println("End of query after JSON sending \n\n");
  }

  if (sens.equals("adc")) {
    Serial.println("ADC measurement..");

    DynamicJsonDocument doc(1024);
    analogReadResolution(12);
    doc["sensor"] = "ADC";
    doc["time"]   = millis();
    doc["measure"] = "voltage";
    doc["data"] = analogRead(27);
    Serial.println("Data sent!");
    serializeJson(doc, query);

    query.stop();
    Serial.println("End of query after JSON sending \n\n");
  }
}
