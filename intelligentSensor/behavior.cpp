/*
   Behavior

   Yann BLANC

  Lists all the modes provided by the
  Intelligent Sensor, and handle the behavior
  it should have depending on the selected mode.

*/


#include "behavior.h"



#define SHT35_ADDRESS   0x45
SHT31 sht;



int nbDevices = 0;

activeBehaviorsList activeBehaviors;
fsActiveBehaviorsList fsActiveBehaviors;


DynamicJsonDocument fsBehavior(1024);

/*
  void configurationSave(){

  File conf=LittleFS.open("configuration.txt","w")
  if(conf){

  }
  }*/


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

  while (measureStarted) {
    Serial.println("PLEASE WAIT");
  }
  measureStarted = true;

  Serial.println("  |TIMER| Measuring requested");

  behavior usedBehavior;

  // usedBehavior.sensorScan();
  Serial.println("  |TIMER| Activation");
  float data;
  float time;
  struct Measure mes;
  int dateRef;
  short dbIndex;
  RPI_PICO_Timer* ITimerX;

  int timer = -2;

  if (tim0) {
    dateRef = t0date;
    selTim = 0;
    tim0 = false;
    ITimerX = &ITimer0;
  } else if (tim1) {
    dateRef = t1date;
    selTim = 1;
    tim1 = false;
    ITimerX = &ITimer1;
  } else if (tim2) {
    dateRef = t2date;
    selTim = 2;
    tim2 = false;
    ITimerX = &ITimer2;
  } else if (tim3) {
    dateRef = t3date;
    selTim = 3;
    tim3 = false;
    ITimerX = &ITimer3;
  }

  for (int at = 0; at < activeBehaviors.size(); at++) {
    if (activeBehaviors[at].timer == selTim) {
      timer = at;
      break;
    }
  }
  if (activeBehaviors[timer].timerStart == -2) {
    activeBehaviors[timer].timerStart = millis();
  }

  Serial.print("  |TIMER| Reading Sensor ID ");
  Serial.println(activeBehaviors[timer].id);

  dbIndex = activeBehaviors[timer].databank - 1;

  switch (activeBehaviors[timer].id) {
    case 1:
      {
        if (tempSens) {
          while (mes.data == -1) {
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
                Serial.println(mes.data);
                mes.time = millis() - dateRef;
                Serial.println(activeBehaviors[timer].id);
                break;
              }

            } else {
              data = sht.getRawTemperature() * (175.0 / 65535) - 45;

              mes.data = data;

              mes.time = millis() - dateRef;
              Serial.println("   \\Data failed");
              DynamicJsonDocument doc(1024);

              doc["error"] = "temperature data is not ready";
              JSONResponse(doc);
            }
            shtCooldown == millis();
            Serial.println(activeBehaviors[timer].id);
            break;
          }
        } else {
          Serial.println("   \\Temperature sensor is not connected");
          break;
        }
        break;
      }
    case 2:
      {
        if (humSens) {

          while (mes.data == -1) {
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
            } else {
              data = sht.getRawHumidity() * (100.0 / 65535);
              mes.data = data;
              mes.time = millis() - dateRef;
              Serial.println("   \\Data failed");
            }
            delay(1);
            shtCooldown == millis();
            break;
          }
        }
        else {
          Serial.println("   \\Humidity sensor is not connected");
          break;
        }


        Serial.println(activeBehaviors[timer].id);
        break;
      }
    case 5:
      {
      
        Serial.println("   \\ADC measurement..");
        data = analogRead(27);
        mes.data = data;
        mes.time = millis() - dateRef;

        delay(1);
        break;
      }
  }





  if (activeBehaviors[timer].mode == 2 || activeBehaviors[timer].mode == 3) {

    if (data < activeBehaviors[timer].min || data > activeBehaviors[timer].max) {
      Serial.print("   |Sensor ");
      Serial.print(activeBehaviors[timer].id);
      Serial.println(" has threshold reached");


      //SEND ALERT
      /*  if (activeQuery.connect(knownIP[0], 80)) {
          Serial.println(knownIP[0]);

              HTTP Request sent to the server

                  activeQuery.println(" HTTP/1.1");
                  activeQuery.println("Content-Type: text/html");
                  activeQuery.println("Connection: close");
                  // the connection will be closed after completion of the response
                  activeQuery.println();
                  activeQuery.println("<!DOCTYPE HTML>");
                  activeQuery.println("<html>");

                  activeQuery.print("DEPASSEMENT");
                  activeQuery.println("<br />");
                  activeQuery.println("</html>");
                  delay(10);



        }*/
      /////////////////////////

    }
  }

  if (activeBehaviors[timer].mode != 3)
  {


    if (indexes[dbIndex] < databankSize - 1) {
      if (indexes[dbIndex] < 0)indexes[dbIndex]++;
      databank[dbIndex][indexes[dbIndex]++] = mes;
      Serial.print("   |Storing measure in databank ");
      Serial.print(dbIndex);
      Serial.print(" at index ");
      Serial.print(indexes[dbIndex]);
      Serial.print(", value is ");
      Serial.println(mes.data);
      Serial.print("\n");
    } else {

      Serial.print("   |Databank ");
      Serial.print(dbIndex);
      Serial.println(" is full\n ");

    }
  }


  measureStarted = false;
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
    doc["activeBehaviors"][ab]["maximum"] = activeBehaviors[ab].max;
    doc["activeBehaviors"][ab]["minimum"] = activeBehaviors[ab].min;
    doc["activeBehaviors"][ab]["databank"] = activeBehaviors[ab].databank;
    doc["activeBehaviors"][ab]["timer"] = activeBehaviors[ab].timer + 1;
  }


  doc["lastBehavior"]["mode"] = lastBehavior.mode;
  doc["lastBehavior"]["sensor"] = lastBehavior.id;
  doc["lastBehavior"]["period"] = lastBehavior.readingPeriod;

  JSONResponse(doc);

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

   MODE 2: REGULAR MEASURE + THRESHOLD WATCHDOG

   Mode 1 with alert sent to the server whenever
   the specified threshold (minimal value and maximal values)
   is passed over.

      IP/?M=2&C=id&P=period(ms)&min=minimumValue&MM=maximumValue

   MODE 3: THRESHOLD WATCHDOG

   Mode 2 without data storage.

      IP/?M=3&C=id&P=period(ms)&min=minimumValue&MM=maximumValue

*/




DynamicJsonDocument fsAB_A(1024);
DynamicJsonDocument fsAB_B(1024);
DynamicJsonDocument fsAB_C(1024);
DynamicJsonDocument fsAB_D(1024);

DynamicJsonDocument fsAB_a(1024);
DynamicJsonDocument fsAB_b(1024);

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
    JSONResponse(doc);
  } else if (activeCommand.readingPeriod * 1000 >= 1e6 && activeCommand.readingPeriod * 1000 <= 3600e6) {

    Serial.print("Timer initialisation ");


    if (activeCommand.id < 0) {
      DynamicJsonDocument doc(1024);

      doc["error"] = "sensor id is not specified";
      JSONResponse(doc);

    } else  if (indexes[0] > 0 && indexes[1] > 0 && indexes[2] > 0 && indexes[3] > 0) {
      DynamicJsonDocument doc(1024);

      doc["error"] = "no databank available";
      JSONResponse(doc);
    } else if ((activeCommand.mode == 2 && activeCommand.min == -1 && activeCommand.max == -1) || (activeCommand.mode == 3 && activeCommand.min == -1 && activeCommand.max == -1)) {
      DynamicJsonDocument doc(1024);

      doc["error"] = "no threshold specified";
      JSONResponse(doc);
    }
    else {




      if (activeCommand.id > 2 || ((activeCommand.id == 1 && millis() - shtCooldown > 20 && sht.dataReady()) || (activeCommand.id == 2 && millis() - shtCooldown > 20 && sht.dataReady()))) {

        for (int i = 0 ; i < activeBehaviors.size(); i++) {
          if (activeBehaviors[i].timerStart > 0) {
            int a = (millis() - activeBehaviors[i].timerStart) % activeBehaviors[i].readingPeriod;
            if (a > activeBehaviors[i].readingPeriod - 15 || a < 15) {
              readyToStartTimer = false;
              break;
            }
          }
          readyToStartTimer = true;
        }

        if (readyToStartTimer) {

          if (activeCommand.mode != 3) {
            for (int i = 0; i < 4; i++) {
              if (indexes[i] < 0) {
                activeCommand.databank = i + 1;
                indexes[i]++;

                break;
              }
            }
          }

          Serial.print("in databank ");
          Serial.println(activeCommand.databank);

          if (!timer0Used) {
            if (ITimer0.attachInterruptInterval(activeCommand.readingPeriod * 1000, behavior::timer0Handler)) {

              Serial.println("Timer Start OK");
              ITimer0.restartTimer();
              t0date = millis();
              timer0Used = true;
              activeCommand.timer = 0;
              lastBehavior = activeCommand;

              activeBehaviors.push_back(activeCommand);


              fsAB_A["mode"] = activeCommand.mode;
              fsAB_A["id"] = activeCommand.id;
              fsAB_A["timer"] = "0";
              fsAB_A["readingPeriod"] = activeCommand.readingPeriod;

              fsActiveBehaviors.push_back(&fsAB_A);
              ////configurationSave();
              /*
                File f = LittleFS.open("activeBehaviors.txt", "w");
                if (f) {

                fsLittleBehavior["timer"] = "0";
                Serial.println("ECRITURE");
                serializeJson(fsLittleBehavior, f);
                f.close();
                }*/
              DynamicJsonDocument doc(1024);
              doc["status"] = "timer 1 started";
              JSONResponse(doc);

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
              /*

                            File f = LittleFS.open("activeBehaviors.txt", "w");
                            if (f) {

                              fsLittleBehavior["timer"] = "1";
                              fsLittleBehavior["readingPeriod"]= readingPeriod;

                              Serial.println("ECRITURE");
                              serializeJson(fsLittleBehavior, f);
                              f.close();
                            }*/

              fsAB_B["mode"] = activeCommand.mode;
              fsAB_B["id"] = activeCommand.id;
              fsAB_B["timer"] = "1";
              fsAB_B["readingPeriod"] = activeCommand.readingPeriod;

              fsActiveBehaviors.push_back(&fsAB_B);
              //configurationSave();


              DynamicJsonDocument doc(1024);
              doc["status"] = "timer 2 started";
              JSONResponse(doc);

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


              fsAB_C["mode"] = activeCommand.mode;
              fsAB_C["id"] = activeCommand.id;
              fsAB_C["timer"] = "2";
              fsAB_C["readingPeriod"] = activeCommand.readingPeriod;

              fsActiveBehaviors.push_back(&fsAB_C);
              //configurationSave();

              /*   File f = LittleFS.open("activeBehaviors.txt", "w");
                 if (f) {

                   fsLittleBehavior["timer"] = "3";
                   Serial.println("ECRITURE");
                   serializeJson(fsLittleBehavior, f);
                   f.close();
                 }
              */
              DynamicJsonDocument doc(1024);
              doc["status"] = "timer 3 started";
              JSONResponse(doc);

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

              txtActiveBehavior += "t=3\n";


              fsAB_D["mode"] = activeCommand.mode;
              fsAB_D["id"] = activeCommand.id;
              fsAB_D["timer"] = "3";
              fsAB_D["readingPeriod"] = activeCommand.readingPeriod;

              fsActiveBehaviors.push_back(&fsAB_D);
              //configurationSave();
              /*
                            File f = LittleFS.open("activeBehaviors.txt", "w");
                            if (f) {

                              fsLittleBehavior["timer"] = "3";
                              Serial.println("ECRITURE");
                              serializeJson(fsLittleBehavior, f);
                              f.close();
                            }*/
              DynamicJsonDocument doc(1024);
              doc["status"] = "timer 4 started";
              JSONResponse(doc);

            } else {
              Serial.println("Failed to start timer");
            }
          }

        } else {

          Serial.println("\nOverlap (before)");
          DynamicJsonDocument doc(1024);
          doc["status"] = "timer could not start because of measure overlap (before). Please try again.";
          JSONResponse(doc);

        }
      } else {
        Serial.println("\nOverlap (after)");
        DynamicJsonDocument doc(1024);
        doc["status"] = "timer could not start because of measure overlap (after). Please try again.";
        JSONResponse(doc);
      }
    }

  } else {
    DynamicJsonDocument doc(1024);

    doc["error"] = "reading period is out of bonds";
    JSONResponse(doc);


  }

  txtActiveBehavior = " ";
}

/*MODE 4 : DETECTION


  Works on pin 3 & 4

  P=0 -> Send an alert whenever the pin read value changes.
  P>0 -> Same as previous mode, but the P value defines the alert
  cooldown to prevent overflowing requests.

        IP/?M=4&C=sensors&P=cooldown

*/
void behavior::detectionAction() {

  int interruptPin = -2;

  bool* irXused;
  int* startX;

  if (itA) {
    startX = &startA;
    irXused = &irAused;
    for (int at = 0; at < activeBehaviors.size(); at++) {
      if (activeBehaviors[at].mode == 4) {

        if (activeBehaviors[at].id == PIN_A) {
          interruptPin = at;
          break;
        }
      }
    }


  } else if (itB) {
    startX = &startB;
    irXused = &irBused;
    for (int at = 0; at < activeBehaviors.size(); at++) {
      if (activeBehaviors[at].mode == 4) {

        if (activeBehaviors[at].id == PIN_B) {
          interruptPin = at;
          break;
        }
      }
    }



  }
  if (activeBehaviors[interruptPin].readingPeriod > 0 && (millis() - *startX) > activeBehaviors[interruptPin].readingPeriod) {


    Serial.println("DETECTION");

    //SEND ALERT
    if (activeQuery.connect(knownIP[0], 80)) {
      Serial.println(knownIP[0]);

      // HTTP Request sent to the server

      activeQuery.println(" HTTP/1.1");
      activeQuery.println("Content-Type: text/html");
      activeQuery.println("Connection: close");
      // the connection will be closed after completion of the response
      activeQuery.println();
      activeQuery.println("<!DOCTYPE HTML>");
      activeQuery.println("<html>");

      activeQuery.print("DEPASSEMENT");

      activeQuery.println("<br />");
      activeQuery.println("</html>");
      delay(10);



    }
    /////////////////////////




    *startX = millis();
  }
  if (activeBehaviors[interruptPin].readingPeriod <= 0) {
    Serial.println("DETECTION");
    detachInterrupt(digitalPinToInterrupt(activeBehaviors[interruptPin].id));
    *irXused = false;
    activeBehaviors.remove(interruptPin);
  }

}



void behavior::detectionAHandler() {
  itA = true;
  behavior::detectionAction();
}

void behavior::detectionBHandler() {
  itB = true;
  behavior::detectionAction();
}






void behavior::detection() {

  if (activeCommand.id < 0) {


    DynamicJsonDocument doc(1024);

    doc["error"] = "sensor id is not specified";
    JSONResponse(doc);
  } else if (activeCommand.id == PIN_A || activeCommand.id == PIN_B) {


    if (activeCommand.id == PIN_A ) {
      if (!irAused) {
        attachInterrupt(digitalPinToInterrupt(PIN_A), detectionAHandler, CHANGE);

        /* timer0Used = true;
          activeCommand.timer = 0;*/
        lastBehavior = activeCommand;

        activeBehaviors.push_back(activeCommand);


        fsAB_a["mode"] = activeCommand.mode;
        fsAB_a["id"] = activeCommand.id;
        fsActiveBehaviors.push_back(&fsAB_a);
        //configurationSave();

        DynamicJsonDocument doc(1024);
        doc["status"] = "interrupt A started";
        JSONResponse(doc);

        Serial.println("Interrupt set on sensor ");
        Serial.println(activeCommand.id);
        irAused = true;
      } else {
        DynamicJsonDocument doc(1024);

        doc["error"] = "interrupt on pin A already started";
        JSONResponse(doc);
      }
    } else if (activeCommand.id == PIN_B ) {
      if (!irBused) {
        attachInterrupt(digitalPinToInterrupt(PIN_B), detectionBHandler, CHANGE);

        /* timer0Used = true;
          activeCommand.timer = 0;*/
        lastBehavior = activeCommand;

        activeBehaviors.push_back(activeCommand);

        fsAB_b["mode"] = activeCommand.mode;
        fsAB_b["id"] = activeCommand.id;
        fsActiveBehaviors.push_back(&fsAB_b);
        //configurationSave();


        DynamicJsonDocument doc(1024);
        doc["status"] = "interrupt B started";
        JSONResponse(doc);

        Serial.println("Interrupt set on sensor ");
        Serial.println(activeCommand.id);

        irBused = true;

      } else {
        DynamicJsonDocument doc(1024);
        doc["error"] = "interrupt on pin 4 already started";
        JSONResponse(doc);
      }

    }
  } else {
    DynamicJsonDocument doc(1024);

    doc["error"] = "sensor id is not compatible with mode 4";
    JSONResponse(doc);
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
        JSONResponse(doc);


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
        JSONResponse(doc);


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
        JSONResponse(doc);

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

  Measure clearMes;


  if (!(activeCommand.databank < 0)) {

    DynamicJsonDocument doc(65536);




    for (int dataRead = 0; dataRead < indexes[activeCommand.databank - 1]; dataRead++) {

      Serial.println(databank[activeCommand.databank - 1][dataRead].data);
      doc["data"][dataRead] = databank[activeCommand.databank - 1][dataRead].data;
      doc["time"][dataRead] = databank[activeCommand.databank - 1][dataRead].time;


      databank[activeCommand.databank - 1][dataRead] = clearMes;
    }

    indexes[activeCommand.databank - 1] = -2;

    serializeJson(doc, activeQuery);

    activeQuery.stop();

    // JSONResponse(doc);

    /*  DynamicJsonDocument doc(1024);

      doc["status"] = "data sent to server";
      serializeJson(doc,activeQuery);*/

  } else {

    DynamicJsonDocument doc(1024);

    doc["error"] = "databank is not specified";
    JSONResponse(doc);

  }
}





/*MODE 11 : MEASURE RESET

   Stops selected periodic measure
   I=0 stops all the timers
   I=5 to stop Mode 4 set on pin 3
    IP/?M=11&I=timer

*/

void behavior::measureReset() {

  if (!(activeCommand.interrupt < 0)) {

    bool* timerXUsed;
    int timInt;
    RPI_PICO_Timer* ITimerX;



    if (activeCommand.interrupt == 0) {


      ITimer0.stopTimer();
      ITimer1.stopTimer();
      ITimer2.stopTimer();
      ITimer3.stopTimer();

      timer0Used = false;
      timer1Used = false;
      timer2Used = false;
      timer3Used = false;




      fsActiveBehaviors.clear();
      activeBehaviors.clear();
      //configurationSave();
    } else {
      switch (activeCommand.interrupt) {

        case 1:
          {
            ITimerX = &ITimer0;
            timInt = 0;
            timerXUsed = &timer0Used;

            DynamicJsonDocument doc(1024);
            doc["status"] = "timer 0 stopped";
            JSONResponse(doc);

            break;
          }
        case 2:
          {
            ITimerX = &ITimer1;
            timInt = 1;
            timerXUsed = &timer1Used;

            DynamicJsonDocument doc(1024);
            doc["status"] = "timer 1 stopped";
            JSONResponse(doc);

            break;
          }
        case 3:
          {
            ITimerX = &ITimer2;
            timInt = 2;
            timerXUsed = &timer2Used;

            DynamicJsonDocument doc(1024);
            doc["status"] = "timer 2 stopped";
            JSONResponse(doc);
            break;
          }
        case 4:
          {
            ITimerX = &ITimer3;
            timInt = 3;
            timerXUsed = &timer3Used;

            DynamicJsonDocument doc(1024);
            doc["status"] = "timer 3 stopped";
            JSONResponse(doc);
            break;
          }
        case 5:
          {
            irAused = false;
            detachInterrupt(digitalPinToInterrupt(PIN_A));
            for (int cmd = 0; cmd < activeBehaviors.size(); cmd++) {
              if (activeBehaviors[cmd].mode == 4) {
                if (activeBehaviors[cmd].id == PIN_A) {
                  activeBehaviors.remove(cmd);
                  fsActiveBehaviors.remove(cmd);
                  //configurationSave();
                }
              }
            }
            break;
          }
        case 6:
          {
            irBused = false;
            detachInterrupt(digitalPinToInterrupt(PIN_B));
            for (int cmd = 0; cmd < activeBehaviors.size(); cmd++) {
              if (activeBehaviors[cmd].mode == 4) {
                if (activeBehaviors[cmd].id == PIN_B) {
                  activeBehaviors.remove(cmd);
                  fsActiveBehaviors.remove(cmd);
                  //configurationSave();
                }
              }
            }
            break;
          }
      }

      for (int cmd = 0; cmd < activeBehaviors.size(); cmd++) {
        if (activeBehaviors[cmd].timer == timInt) {
          ITimerX->stopTimer();
          *timerXUsed = false;
          activeBehaviors.remove(cmd);
          fsActiveBehaviors.remove(cmd);
          //configurationSave();
        }

      }


    }
  }
  else {
    DynamicJsonDocument doc(1024);
    int a = 2;
    doc["error" + String(a)] = "measure to stop is not specified";
    JSONResponse(doc);

  }
}


void behavior::checkPreviousConfiguration(struct Command* cmdA, struct Command* cmdB, struct Command* cmdC, struct Command* cmdD, struct Command* cmdE, struct Command* cmdF) {
  String str;
  File f = LittleFS.open("activeBehaviors.txt", "r");
  DynamicJsonDocument savedConfiguration(8192);
  if (f) {
    /*while (f.available()) {
      str += f.read();
      Serial.println(str);
      }*/
    deserializeJson(savedConfiguration, f);





    /*int mode=-1;
      int id=-1;
      int readingPeriod=-1;
      int min=-1;
      int max=-1;
      int logicalLevel=-1;
      int databank=-1;
      int timer=-1;
      int timerStart=-2;
      int interrupt=-1;*/

    cmdA->mode = savedConfiguration["command1"]["mode"];
    cmdA->id = savedConfiguration["command1"]["id"];
    cmdA->readingPeriod = savedConfiguration["command1"]["readingPeriod"];
    cmdA->min = savedConfiguration["command1"]["min"];
    cmdA->max = savedConfiguration["command1"]["max"];
    cmdA->logicalLevel = savedConfiguration["command1"]["logicalLevel"];
    cmdA->databank = savedConfiguration["command1"]["databank"];
    cmdA->timer = savedConfiguration["command1"]["mode"];
    cmdA->timerStart = savedConfiguration["command1"]["timerStart"];
    cmdA->interrupt = savedConfiguration["command1"]["interrupt"];


    cmdB->mode = savedConfiguration["command2"]["mode"];
    cmdB->id = savedConfiguration["command2"]["id"];
    cmdB->readingPeriod = savedConfiguration["command2"]["readingPeriod"];
    cmdB->min = savedConfiguration["command2"]["min"];
    cmdB->max = savedConfiguration["command2"]["max"];
    cmdB->logicalLevel = savedConfiguration["command2"]["logicalLevel"];
    cmdB->databank = savedConfiguration["command2"]["databank"];
    cmdB->timer = savedConfiguration["command2"]["mode"];
    cmdB->timerStart = savedConfiguration["command2"]["timerStart"];
    cmdB->interrupt = savedConfiguration["command2"]["interrupt"];

    cmdC->mode = savedConfiguration["command3"]["mode"];
    cmdC->id = savedConfiguration["command3"]["id"];
    cmdC->readingPeriod = savedConfiguration["command3"]["readingPeriod"];
    cmdC->min = savedConfiguration["command3"]["min"];
    cmdC->max = savedConfiguration["command3"]["max"];
    cmdC->logicalLevel = savedConfiguration["command3"]["logicalLevel"];
    cmdC->databank = savedConfiguration["command3"]["databank"];
    cmdC->timer = savedConfiguration["command3"]["mode"];
    cmdC->timerStart = savedConfiguration["command3"]["timerStart"];
    cmdC->interrupt = savedConfiguration["command3"]["interrupt"];

    cmdD->mode = savedConfiguration["command4"]["mode"];
    cmdD->id = savedConfiguration["command4"]["id"];
    cmdD->readingPeriod = savedConfiguration["command4"]["readingPeriod"];
    cmdD->min = savedConfiguration["command4"]["min"];
    cmdD->max = savedConfiguration["command4"]["max"];
    cmdD->logicalLevel = savedConfiguration["command4"]["logicalLevel"];
    cmdD->databank = savedConfiguration["command4"]["databank"];
    cmdD->timer = savedConfiguration["command4"]["mode"];
    cmdD->timerStart = savedConfiguration["command4"]["timerStart"];
    cmdD->interrupt = savedConfiguration["command4"]["interrupt"];

    cmdE->mode = savedConfiguration["command5"]["mode"];
    cmdE->id = savedConfiguration["command5"]["id"];
    cmdE->readingPeriod = savedConfiguration["command5"]["readingPeriod"];
    cmdE->min = savedConfiguration["command5"]["min"];
    cmdE->max = savedConfiguration["command5"]["max"];
    cmdE->logicalLevel = savedConfiguration["command5"]["logicalLevel"];
    cmdE->databank = savedConfiguration["command5"]["databank"];
    cmdE->timer = savedConfiguration["command5"]["mode"];
    cmdE->timerStart = savedConfiguration["command5"]["timerStart"];
    cmdE->interrupt = savedConfiguration["command5"]["interrupt"];

    cmdF->mode = savedConfiguration["command6"]["mode"];
    cmdF->id = savedConfiguration["command6"]["id"];
    cmdF->readingPeriod = savedConfiguration["command6"]["readingPeriod"];
    cmdF->min = savedConfiguration["command6"]["min"];
    cmdF->max = savedConfiguration["command6"]["max"];
    cmdF->logicalLevel = savedConfiguration["command6"]["logicalLevel"];
    cmdF->databank = savedConfiguration["command6"]["databank"];
    cmdF->timer = savedConfiguration["command6"]["mode"];
    cmdF->timerStart = savedConfiguration["command6"]["timerStart"];
    cmdF->interrupt = savedConfiguration["command6"]["interrupt"];

    Serial.println("---------------");
    f.close();
  }
}


void behavior::configurationSave() {
  int i = 0;
  LittleFS.remove("activeBehaviors.txt");
  File f = LittleFS.open("activeBehaviors.txt", "w");
  DynamicJsonDocument configurationToSave(8192);

  for (DynamicJsonDocument *fsAB : fsActiveBehaviors) {
    i++;
    configurationToSave["command" + String(i)] = *fsAB;
  }
  serializeJson(configurationToSave, f);
  f.close();
}


void behavior::JSONResponse(DynamicJsonDocument resp) {
  activeQuery.println(Http_xHeader);
  serializeJson(resp, activeQuery);
  delay(10);
  activeQuery.stop();
}

void behavior::behaviorHandler(struct Command command, EthernetClient query, EthernetClient client) {

  activeCommand = command;
  activeQuery = query;
  activeClient = client;

  IPAddress nullIP(0, 0, 0, 0);

  if (activeQuery.remoteIP() != nullIP) {
    knownIP[0] = activeQuery.remoteIP();
  }

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
          txtActiveBehavior += "m=1";
          regularMeasure();
          break;
        }
      case 2:
        {
          Serial.print(">Selected mode: ");
          txtActiveBehavior += "m=2";
          regularMeasure();
          break;
        }
      case 3:
        {
          Serial.print(">Selected mode: ");
          txtActiveBehavior += "m=3";
          regularMeasure();
          break;
        }
      case 4:
        {
          Serial.print(">Selected mode: ");
          txtActiveBehavior += "m=4";
          detection();
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
    JSONResponse(doc);

  }
}
