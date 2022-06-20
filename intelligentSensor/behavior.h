/*
   Behavior

   Yann BLANC

  Lists all the modes provided by the
  Intelligent Sensor, and handle the behavior
  it should have depending on the selected mode.
  
*/

#ifndef behavior_H
#define behavior_H

#include <Arduino.h>
#include "Wire.h"
#include "SHT31.h"
#include <ArduinoJson.h>
#include <Ethernet.h>
#include "Command.h"
#include "Measure.h"
#include "ISnetwork.h"
#include <Array.h>
#include <LittleFS.h>
#include "Sensor.h"


#define TIMER_INTERRUPT_DEBUG         1
#define _TIMERINTERRUPT_LOGLEVEL_     4

#pragma once
#include "RPi_Pico_TimerInterrupt.h"
#include "RPi_Pico_ISR_Timer.hpp"

#define LOCAL_DEBUG               1

typedef Array<struct Command, 6> activeBehaviorsList;
typedef Array<DynamicJsonDocument*, 6> fsActiveBehaviorsList;
typedef Array<struct Sensor, 6> sensorList;
static const short databankSize = 3600;

class behavior  {

  private:
  
#define PIN_A 12
#define PIN_B 13

  public:
  
  
  
    static void measure();
    static bool timer0Handler(struct repeating_timer *t);
    static bool timer1Handler(struct repeating_timer *t);
    static bool timer2Handler(struct repeating_timer *t);
    static bool timer3Handler(struct repeating_timer *t);

    static void detectionAction();
    static void detectionAHandler();
    static void detectionBHandler();

    short int nbDevices;
    int connectedSensors[128];
    void sensorScan();





    void sensorSetup();
    void behaviorHandler(struct Command activeCommand, EthernetClient activeQuery, EthernetClient client);
    void ISinfo();
    void regularMeasureAlert();
    void regularMeasureAlertDataLess();
    void detection();
    void unitaryMeasure();
    void dataBankRead();
    void behaviourStop();
    void regularMeasure();
    void measureReset();
    void databankRead();
    
    static void JSONResponse(DynamicJsonDocument);
    void configurationSave();
    void checkPreviousConfiguration(struct Command*,struct Command*,struct Command*,struct Command*,struct Command*,struct Command*);
    
    //128716 (4 tableaux)
    //157516 (5 tableaux)
};


static String txtActiveBehavior;

static bool measureStarted;
static EthernetClient activeClient;
static EthernetClient activeQuery;

static bool tempSens=false, humSens=false;

static IPAddress knownIP[10];

static struct Command activeCommand, lastBehavior;
static struct Measure databank[4][databankSize];

static short  idxA = -1, idxB = -1, idxC = -1, idxD = -1;
static short* idxX;
static short indexes[] = {idxA, idxB, idxC, idxD};

static int activeBehaviorsCount;

static bool timer0Used = 0, timer1Used = 0, timer2Used = 0, timer3Used = 0;

static bool readyToStartTimer=true;


static int startA,startB;
static int t0date, t1date, t2date, t3date;
static int selTim;
static bool tim0 = 0, tim1 = 0, tim2 = 0, tim3 = 0;
static RPI_PICO_Timer ITimer0(0);
static RPI_PICO_Timer ITimer1(1);
static RPI_PICO_Timer ITimer2(2);
static RPI_PICO_Timer ITimer3(3);
static int shtCooldown;
static bool irAused = 0, irBused = 0;
static bool itA = 0, itB = 0;

static char Http_xHeader[] = "HTTP/1.1 200 OK \nContent-Type: application/json \nAccess-Control-Allow-Origin: * \nServer: Custom PICO \n";
#endif
