#include "behavior.h"



#define SHT35_ADDRESS   0x45
SHT31 sht;


int idx = 0;
int nbDevices = 0;



//EthernetClient activeQuery;
//struct Command activeBehavior,lastBehavior, activeCommand;
activeBehaviorsList activeBehaviors;


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




bool behavior::timer0Handler(struct repeating_timer *t) {
  tim0 = true;
  behavior::measure();
  return true;
}

bool behavior::timer1Handler(struct repeating_timer *t) {
  tim1 = true;
  behavior::measure();
  return true;
}

bool behavior::timer2Handler(struct repeating_timer *t) {
  tim2 = true;
  behavior::measure();
  return true;
}

bool behavior::timer3Handler(struct repeating_timer *t) {
  tim3 = true;
  behavior::measure();
  return true;
}






void behavior::measure() {

  Serial.println("Measuring requested");

  behavior usedBehavior;

  Serial.println("TIMER| Reading ID");
  Serial.println(activeBehaviors[0].id);
  Serial.println("TIMER| Activation");
  float data;
  float time;
  struct Measure mes;
  databank* db;
  int timer = -2;
  if (tim0) {
    selTim = 0;
    tim0 = false;
  } else if (tim1) {
    selTim = 1;
    tim1 = false;
  } else if (tim2) {
    selTim = 2;
    tim2 = false;
  } else if (tim3) {
    selTim = 3;
    tim3 = false;
  }

  for (int at = 0; at < activeBehaviors.size(); at++) {
    if (activeBehaviors[at].timer == selTim) {
      timer = at;
      break;
    }
  }
  switch (activeBehaviors[timer].databank) {
    case 1:
      db = &dataA;
      break;
    case 2:
      db = &dataB;
      break;
    case 3:
      db = &dataC;
      break;
    case 4:
      db = &dataD;
      break;
  }

  switch (activeBehaviors[timer].id) {
    case 1:
      {

        Serial.println(" >Temperature measurement..");

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
            data = sht.getRawTemperature() * (175.0 / 65535) - 45;
            mes.data = data;
            mes.time = millis();
            db->push_back(mes);
            Serial.println(db->size());
          }
        }
        break;
      }
    case 2:
      {
        Serial.println(" >Humidity measurement..");


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
            data = sht.getRawHumidity() * (100.0 / 65535);
            mes.data = data;
            mes.time = millis();
            db->push_back(mes);
          }
        }

        break;
      }
    case 5:
      {
        Serial.println(" >ADC measurement..");
        data = analogRead(27);
        mes.data = data;
        mes.time = millis();
        db->push_back(mes);

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

  for (int ab = 0; ab <   activeBehaviors.size(); ab++) {
    doc["activeBehavior"][ab]["mode"] = activeBehaviors[ab].mode;
    doc["activeBehavior"][ab]["sensor"] = activeBehaviors[ab].id;
    doc["activeBehavior"][ab]["period"] = activeBehaviors[ab].readingPeriod;
    doc["activeBehavior"][ab]["databank"] = activeBehaviors[ab].databank;
    doc["activeBehavior"][ab]["timer"] = activeBehaviors[ab].timer;
  }


  doc["lastBehavior"]["mode"] = lastBehavior.mode;
  doc["lastBehavior"]["sensor"] = lastBehavior.id;
  doc["lastBehavior"]["period"] = lastBehavior.readingPeriod;

  serializeJson(doc, activeQuery);

  activeQuery.stop();
  Serial.println("End of query after JSON sending \n\n");
}


/*MODE 1 : REGULAR MEASURE

   Using the 4 available timers, periodic
   measures are made on selected sensors,
   from once a second to once an hour.
   Data is stored into an avalaible data bank
   which is then accessible.
*/



void behavior::regularMeasure() {


  Serial.println("REGULAR MEASURE");


  if (activeCommand.readingPeriod < 0) {
    DynamicJsonDocument doc(1024);

    doc["error"] = "reading period is not specified";
    serializeJson(doc, activeQuery);
  } else if (activeCommand.readingPeriod * 1000 >= 1e6 && activeCommand.readingPeriod * 1000 <= 3600e6) {
    Serial.println("Initialisation timer");


    if (activeCommand.id < 0) {
      DynamicJsonDocument doc(1024);

      doc["error"] = "sensor id is not specified";
      serializeJson(doc, activeQuery);

    } else  if (dataA.size() > 0 && dataB.size() > 0 && dataC.size() > 0 && dataD.size() > 0) {
      DynamicJsonDocument doc(1024);

      doc["error"] = "no databank available";
      serializeJson(doc, activeQuery);
    }
    else {

      if (!dataA.size()) {
        Serial.println(dataA.size());
        activeCommand.databank = 1;
      } else if (!dataB.size()) {
        activeCommand.databank = 2;
      } else if (!dataC.size()) {
        activeCommand.databank = 3;
      } else if (!dataD.size()) {
        activeCommand.databank = 4;
      }


      if (!timer0Used) {
        if (ITimer0.attachInterruptInterval(activeCommand.readingPeriod * 1000, behavior::timer0Handler)) {

          Serial.println("Timer Start OK");
          ITimer0.restartTimer();
          timer0Used = true;
          activeCommand.timer = 0;
          lastBehavior = activeCommand;

          activeBehaviors.push_back(activeCommand);

        } else {
          Serial.println("Failed to start timer");
        }
      }
      else if (!timer1Used) {
        if (ITimer1.attachInterruptInterval(activeCommand.readingPeriod * 1000, behavior::timer1Handler)) {
          Serial.println("Timer Start OK");
          ITimer1.restartTimer();
          timer1Used = true;
          activeCommand.timer = 1;
          lastBehavior = activeCommand;
          activeBehaviors.push_back(activeCommand);

        } else {
          Serial.println("Failed to start timer");
        }
      } else if (!timer2Used) {
        if (ITimer2.attachInterruptInterval(activeCommand.readingPeriod * 1000, behavior::timer2Handler)) {
          Serial.println("Timer Start OK");
          ITimer2.restartTimer();
          timer2Used = true;
          activeCommand.timer = 2;
          lastBehavior = activeCommand;
          activeBehaviors.push_back(activeCommand);
        } else {
          Serial.println("Failed to start timer");
        }
      } else if (!timer3Used) {
        if (ITimer3.attachInterruptInterval(activeCommand.readingPeriod * 1000, behavior::timer3Handler)) {
          Serial.println("Timer Start OK");
          ITimer3.restartTimer();
          timer3Used = true;
          activeCommand.timer = 3;
          lastBehavior = activeCommand;
          activeBehaviors.push_back(activeCommand);
        } else {
          Serial.println("Failed to start timer");
        }
      }

    }



  } else {
    DynamicJsonDocument doc(1024);

    doc["error"] = "reading period is out of bonds";
    serializeJson(doc, activeQuery);


  }

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


/*MODE 10 : DATABANK READ
 * 
 *
 *  Provide data from the selected databank
 * 
 *
 */
 
void behavior::databankRead() {

  switch (activeCommand.databank) {
    case 1:
      for (int dataRead = 0; dataRead < dataA.size(); dataRead++) {
        activeQuery.write(dataA[dataRead].data);
      }
      break;
    case 2:

      for (int dataRead = 0; dataRead < dataB.size(); dataRead++) {
        Serial.println(dataB[dataRead].data);
      }
      break;
    case 3:

      for (int dataRead = 0; dataRead < dataC.size(); dataRead++) {
        Serial.println(dataC[dataRead].data);
      }
      break;
    case 4:

      for (int dataRead = 0; dataRead < dataD.size(); dataRead++) {
        Serial.println(dataD[dataRead].data);
      }
      break;
  }


  activeQuery.stop();
}

/*MODE 11 : MEASURE RESET

   Stops selected periodic measure

*/

void behavior::measureReset() {



  switch (activeCommand.interrupt) {
    case 0:
      {
        ITimer0.stopTimer();
        ITimer1.stopTimer();
        ITimer2.stopTimer();
        ITimer3.stopTimer();

        dataA.clear();
        dataB.clear();
        dataC.clear();
        dataD.clear();

        activeBehaviors.clear();
        break;
      }
    case 1:
      {
        for (int cmd = 0; cmd < activeBehaviors.size(); cmd++) {
          if (activeBehaviors[cmd].timer == 1) {
            ITimer0.stopTimer();
            timer0Used = false;
            activeBehaviors.remove(cmd);

            switch (activeBehaviors[cmd].databank) {
              case 1:
                dataA.clear();
                break;
              case 2:
                dataB.clear();
                break;
              case 3:
                dataC.clear();
                break;
              case 4:
                dataD.clear();
                break;
            }
          }

        }
        break;
      }
    case 2:
      {
        for (int cmd = 0; cmd < activeBehaviors.size(); cmd++) {
          if (activeBehaviors[cmd].timer == 1) {
            ITimer1.stopTimer();

            timer1Used = false;
            activeBehaviors.remove(cmd);
            switch (activeBehaviors[cmd].databank) {
              case 1:
                dataA.clear();
                break;
              case 2:
                dataB.clear();
                break;
              case 3:
                dataC.clear();
                break;
              case 4:
                dataD.clear();
                break;
            }
          }

        }
        break;
      }
    case 3:
      {
        for (int cmd = 0; cmd < activeBehaviors.size(); cmd++) {
          if (activeBehaviors[cmd].timer == 2) {
            ITimer2.stopTimer();
            timer2Used = false;
            activeBehaviors.remove(cmd);

            switch (activeBehaviors[cmd].databank) {
              case 1:
                dataA.clear();
                break;
              case 2:
                dataB.clear();
                break;
              case 3:
                dataC.clear();
                break;
              case 4:
                dataD.clear();
                break;
            }
          }

        }
        break;
      }
    case 4:
      {
        for (int cmd = 0; cmd < activeBehaviors.size(); cmd++) {
          if (activeBehaviors[cmd].timer == 3) {
            ITimer3.stopTimer();
            timer3Used = false;
            activeBehaviors.remove(cmd);
            switch (activeBehaviors[cmd].databank) {
              case 1:
                dataA.clear();
                break;
              case 2:
                dataB.clear();
                break;
              case 3:
                dataC.clear();
                break;
              case 4:
                dataD.clear();
                Serial.println("Data D erased");
                break;
            }
          }

        }
        break;
      }

  }
}

void behavior::behaviorHandler(struct Command command, EthernetClient query) {

  activeCommand = command;
  activeQuery = query;

  switch (activeCommand.mode) {
      Serial.print(">Selected mode: ");
    case 0:
      {
        ISinfo();
        break;
      }
    case 1:
      {
        ;
        regularMeasure();
        break;
      }
    case 9:
      {
        unitaryMeasure();
        break;
      }
    case 10:

      {
        databankRead();
        break;
      }
    case 11:
      {

        measureReset();
        break;
      }
  }

}
