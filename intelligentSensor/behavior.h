#ifndef behavior_H
#define behavior_H

#include <Arduino.h>
#include "Wire.h"
#include "SHT31.h"
#include <ArduinoJson.h>
#include <Ethernet.h>
#include "Command.h"
#include "Measure.h"
#include <Array.h>


#define TIMER_INTERRUPT_DEBUG         1
#define _TIMERINTERRUPT_LOGLEVEL_     4

#pragma once
#include "RPi_Pico_TimerInterrupt.h"
#include "RPi_Pico_ISR_Timer.hpp"

#define LOCAL_DEBUG               1

typedef Array<struct Command, 4> activeBehaviorsList;


static struct Measure mesAtest;

class behavior  {

  private:


  public:
    static void measure();
    static bool timer0Handler(struct repeating_timer *t);
    static bool timer1Handler(struct repeating_timer *t);
    static bool timer2Handler(struct repeating_timer *t);
    static bool timer3Handler(struct repeating_timer *t);

    short int nbDevices;
    int connectedSensors[128];
    void sensorScan();

    EthernetClient activeQuery;



    void sensorSetup();
    void behaviorHandler(struct Command activeCommand, EthernetClient activeQuery);
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
    //27960 avec un tableau
    //88872 avec 4



//128716 (4 tableaux)
//157516 (5 tableaux)
};


static struct Command activeCommand, lastBehavior;
static struct Measure databank[5][3600];

static short dbIndex=-1, idxA=-1, idxB=-1, idxC=-1, idxD=-1;
static short* idxX;
static short indexes[] ={idxA,idxB,idxC,idxD};

static int activeBehaviorsCount;

static bool timer0Used = 0, timer1Used = 0, timer2Used = 0, timer3Used = 0;
static int t0date, t1date, t2date, t3date;
static int selTim;
static bool tim0 = 0, tim1 = 0, tim2 = 0, tim3 = 0;
static RPI_PICO_Timer ITimer0(0);
static RPI_PICO_Timer ITimer1(1);
static RPI_PICO_Timer ITimer2(2);
static RPI_PICO_Timer ITimer3(3);


#endif
