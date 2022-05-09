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
typedef Array<struct Measure, 7200> databank;

class behavior  {

  private:

 

  public:
    static void measure();
    static bool timer0Handler(struct repeating_timer *t);
    static bool timer1Handler(struct repeating_timer *t);
    static bool timer2Handler(struct repeating_timer *t);
    static bool timer3Handler(struct repeating_timer *t);

    int nbDevices;
    int connectedSensors[128];
    void sensorScan();

    EthernetClient activeQuery;
    int idx;


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
    };
    static struct Command activeCommand, lastBehavior;
   
    static int activeBehaviorsCount;
    static bool timer0Used = 0, timer1Used = 0, timer2Used = 0, timer3Used = 0;
    static int selTim;
    static bool tim0 = 0, tim1 = 0, tim2 = 0, tim3 = 0;
    static RPI_PICO_Timer ITimer0(0);
    static RPI_PICO_Timer ITimer1(1);
    static RPI_PICO_Timer ITimer2(2);
    static RPI_PICO_Timer ITimer3(3);

    static databank dataA, dataB, dataC, dataD;

#endif
