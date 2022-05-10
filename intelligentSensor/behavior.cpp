#include "behavior.h"



#define SHT35_ADDRESS   0x45
SHT31 sht;



int nbDevices = 0;

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

  Serial.println("  |TIMER| Measuring requested");

  behavior usedBehavior;

  Serial.print("  |TIMER| Reading Sensor ID ");
  Serial.println(activeBehaviors[0].id);
  Serial.println("  |TIMER| Activation");
  float data;
  float time;
  struct Measure mes;
  int dateRef;



  int timer = -2;

  if (tim0) {
    dateRef = t0date;
    selTim = 0;
    tim0 = false;
  } else if (tim1) {
    dateRef = t1date;
    selTim = 1;
    tim1 = false;
  } else if (tim2) {
    dateRef = t2date;
    selTim = 2;
    tim2 = false;
  } else if (tim3) {
    dateRef = t3date;
    selTim = 3;
    tim3 = false;
  }

  for (int at = 0; at < activeBehaviors.size(); at++) {
    if (activeBehaviors[at].timer == selTim) {
      timer = at;
      break;
    }
  }


  dbIndex = activeBehaviors[timer].databank - 1;

  switch (activeBehaviors[timer].id) {
    case 1:
      {

        Serial.println("   \\Temperature measurement..");

        if (sht.dataReady())
        {
          bool success  = sht.readData();
          sht.requestData();

          if (success == false)
          {
            Serial.println("  \\Failed read");
          }
          else
          {

            data = sht.getRawTemperature() * (175.0 / 65535) - 45;
            mes.data = data;
            mes.time = millis() - dateRef;
          }
        }
        break;
      }
    case 2:
      {
        Serial.println("   \\Humidity measurement..");


        if (sht.dataReady())
        {
          bool success  = sht.readData();
          sht.requestData();

          if (success == false)
          {
            Serial.println("   \\Failed read");
          }
          else
          {
            data = sht.getRawHumidity() * (100.0 / 65535);
            mes.data = data;
            mes.time = millis() - dateRef;


          }
        }

        break;
      }
    case 5:
      {
        Serial.println("   \\ADC measurement..");
        data = analogRead(27);
        mes.data = data;
        mes.time = millis() - dateRef;


        break;
      }
  }

  if (activeBehaviors[timer].mode == 2 || activeBehaviors[timer].mode == 3) {

    if (data < activeBehaviors[timer].min || data > activeBehaviors[timer].max) {
      Serial.print("   |Sensor ");
      Serial.print(activeBehaviors[timer].id);
      Serial.println(" has threshold reached");
    }
  }

  if (activeBehaviors[timer].mode != 3)
  {
    databank[dbIndex][indexes[dbIndex]++] = mes;
    Serial.print("   |Storing measure in databank ");
    Serial.print(dbIndex);
    Serial.print(" at index\n ");
    Serial.println(indexes[dbIndex]);

  }

}



/*MODE 0 : INTELLIGENT SENSOR INFO

   Returns what are the connected sensors,
   what are the active meausre modes ,
   and the last mode selected with the
   associated parameters.

            IP/?M=0

*/
void behavior::ISinfo() {
  Serial.println("INTELLIGENT SENSOR INFO");
  DynamicJsonDocument doc(1024);

  for (int i = 0; i < nbDevices; i++) {
    doc["sensors"][i] = connectedSensors[i];
  }

  for (int ab = 0; ab <   activeBehaviors.size(); ab++) {
    doc["activeBehaviors"][ab]["mode"] = activeBehaviors[ab].mode;
    doc["activeBehaviors"][ab]["sensor"] = activeBehaviors[ab].id;
    doc["activeBehaviors"][ab]["period"] = activeBehaviors[ab].readingPeriod;
    doc["activeBehaviors"][ab]["databank"] = activeBehaviors[ab].databank;
    doc["activeBehaviors"][ab]["timer"] = activeBehaviors[ab].timer + 1;
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

     IP/?M=1&C=id&P=period(ms)

   MODE 2: REGULAR MEASURE + THRESHOLD

   Mode 1 with alert sent to the server whenever
   the specified threshold (minimal value and maximal values)
   is passed over.

      IP/?M=2&C=id&P=period(ms)&min=minimumValue&MM=maximumValue

   MODE 3: THRESHOLD WATCHDOG

   Mode 2 without data storage.

      IP/?M=3&C=id&P=period(ms)&min=minimumValue&MM=maximumValue

*/



void behavior::regularMeasure() {


  switch (activeCommand.mode) {
    case 1:
      Serial.println("REGULAR MEASURE");
      break;
    case 2:
      Serial.println("REGULAR MEASURE w/ TRESHOLD WATCHDOG");
      break;
    case 3:
      Serial.println("TRESHOLD WATCHDOG");
      break;
  }


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

    } else  if (indexes[0] > 0 && indexes[1] > 0 && indexes[2] > 0 && indexes[3] > 0) {
      DynamicJsonDocument doc(1024);

      doc["error"] = "no databank available";
      serializeJson(doc, activeQuery);
    } else if ((activeCommand.mode == 2 && activeCommand.min == -1 && activeCommand.max == -1) || (activeCommand.mode == 3 && activeCommand.min == -1 && activeCommand.max == -1)) {
      DynamicJsonDocument doc(1024);

      doc["error"] = "no threshold specified";
      serializeJson(doc, activeQuery);
    }
    else {

      if (activeCommand.mode != 3) {
        for (int i = 0; i < 4; i++) {
          if (indexes[i] < 0) {
            activeCommand.databank = i + 1;
            indexes[i]++;
            break;
          }
        }
      }


      if (!timer0Used) {
        if (ITimer0.attachInterruptInterval(activeCommand.readingPeriod * 1000, behavior::timer0Handler)) {

          Serial.println("Timer Start OK");
          ITimer0.restartTimer();
          t0date = millis();
          timer0Used = true;
          activeCommand.timer = 0;
          lastBehavior = activeCommand;

          activeBehaviors.push_back(activeCommand);


          DynamicJsonDocument doc(1024);
          doc["status"] = "timer 1 started";
          serializeJson(doc, activeQuery);

        } else {
          Serial.println("Failed to start timer");
        }
      }
      else if (!timer1Used) {
        if (ITimer1.attachInterruptInterval(activeCommand.readingPeriod * 1000, behavior::timer1Handler)) {
          Serial.println("Timer Start OK");
          ITimer1.restartTimer();
          t1date = millis();
          timer1Used = true;
          activeCommand.timer = 1;
          lastBehavior = activeCommand;
          activeBehaviors.push_back(activeCommand);

          DynamicJsonDocument doc(1024);
          doc["status"] = "timer 2 started";
          serializeJson(doc, activeQuery);

        } else {
          Serial.println("Failed to start timer");
        }
      } else if (!timer2Used) {
        if (ITimer2.attachInterruptInterval(activeCommand.readingPeriod * 1000, behavior::timer2Handler)) {
          Serial.println("Timer Start OK");
          ITimer2.restartTimer();
          t2date = millis();
          timer2Used = true;
          activeCommand.timer = 2;
          lastBehavior = activeCommand;
          activeBehaviors.push_back(activeCommand);

          DynamicJsonDocument doc(1024);
          doc["status"] = "timer 3 started";
          serializeJson(doc, activeQuery);

        } else {
          Serial.println("Failed to start timer");
        }
      } else if (!timer3Used) {
        if (ITimer3.attachInterruptInterval(activeCommand.readingPeriod * 1000, behavior::timer3Handler)) {
          Serial.println("Timer Start OK");
          ITimer3.restartTimer();
          t3date = millis();
          timer3Used = true;
          activeCommand.timer = 3;
          lastBehavior = activeCommand;
          activeBehaviors.push_back(activeCommand);

          DynamicJsonDocument doc(1024);
          doc["status"] = "timer 4 started";
          serializeJson(doc, activeQuery);

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


    Provide data from the selected databank

          IP/?M=10&B=databank
          
*/
void behavior::databankRead() {


  Serial.println("DATABANK READ");


  if (!(activeCommand.databank < 0)) {

    for (int dataRead = 0; dataRead < indexes[activeCommand.databank - 1]; dataRead++) {
      Serial.println(databank[activeCommand.databank - 1][dataRead].data);
    }
    DynamicJsonDocument doc(1024);

    doc["status"] = "data sent to server";
    serializeJson(doc, activeQuery);

    activeQuery.stop();
  } else {
    DynamicJsonDocument doc(1024);

    doc["error"] = "databank is not specified";
    serializeJson(doc, activeQuery);

  }
}





/*MODE 11 : MEASURE RESET

   Stops selected periodic measure

    IP/?M=11&I=timer

*/

void behavior::measureReset() {

  if (!(activeCommand.interrupt < 0)) {

    bool* timerXUsed;
    int timInt;
    RPI_PICO_Timer* ITimerX;

    Measure clearMes;

    if (activeCommand.interrupt == 0) {


      ITimer0.stopTimer();
      ITimer1.stopTimer();
      ITimer2.stopTimer();
      ITimer3.stopTimer();

      timer0Used = false;
      timer1Used = false;
      timer2Used = false;
      timer3Used = false;




      for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 3600; i++) {
          databank[j][i] = clearMes;
        }
        indexes[j] = -1;
      }


      
      activeBehaviors.clear();
    } else {
      switch (activeCommand.interrupt) {

        case 1:
          {
            ITimerX = &ITimer0;
            timInt = 0;
            timerXUsed = &timer0Used;
            break;
          }
        case 2:
          {
            ITimerX = &ITimer1;
            timInt = 1;
            timerXUsed = &timer1Used;
            break;
          }
        case 3:
          {
            ITimerX = &ITimer2;
            timInt = 2;
            timerXUsed = &timer2Used;
            break;
          }
        case 4:
          {
            ITimerX = &ITimer3;
            timInt = 3;
            timerXUsed = &timer3Used;
            break;
          }
      }

      for (int cmd = 0; cmd < activeBehaviors.size(); cmd++) {
        if (activeBehaviors[cmd].timer == timInt) {
          ITimerX->stopTimer();
          *timerXUsed = false;
          activeBehaviors.remove(cmd);

          for (int i = 0; i < indexes[timInt]; i++) {
            databank[activeBehaviors[cmd].databank][i] = clearMes;
          }

          indexes[timInt] = -1;
        }

      }


    }
  } else {
    DynamicJsonDocument doc(1024);

    doc["error"] = "measure to stop is not specified";
    serializeJson(doc, activeQuery);

  }
}








void behavior::behaviorHandler(struct Command command, EthernetClient query) {

  activeCommand = command;
  activeQuery = query;

  if (!(activeCommand.mode < 0)) {
    switch (activeCommand.mode) {

      case 0:
        { Serial.print(">Selected mode: ");
          ISinfo();
          break;
        }
      case 1:
        {
          Serial.print(">Selected mode: ");
          regularMeasure();
          break;
        }
      case 2:
        {
          Serial.print(">Selected mode: ");
          regularMeasure();
          break;
        }
      case 3:
        {
          Serial.print(">Selected mode: ");
          regularMeasure();
          break;
        }
      case 9:
        { Serial.print(">Selected mode: ");
          unitaryMeasure();
          break;
        }
      case 10:

        { Serial.print(">Selected mode: ");
          databankRead();
          break;
        }
      case 11:
        {
          Serial.print(">Selected mode: ");
          measureReset();
          break;
        }
    }
  } else {
    DynamicJsonDocument doc(1024);

    doc["error"] = "mode is not specified";
    serializeJson(doc, activeQuery);

  }
}
