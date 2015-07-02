#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <QStateMachine>
#include <QState>
#include <iostream>
#include <string>

#include "userevents.h"
#include "usereventtransition.h"
#include "tptimer.h"

/////////////
// StateBase
// (can print)
/////////////
class StateBase : public QState {
 public:
 StateBase(const std::string &name_, QState * parent = nullptr)                      : QState{parent},            name{name_} {}
 StateBase(const std::string &name_, ChildMode childMode, QState * parent = nullptr) : QState{childMode, parent}, name{name_} {}
    
 protected:
  void onEntry(QEvent */*event*/) {
    std::cout << "Entering: " << name << std::endl;
  }
  void onExit(QEvent */*event*/) {
    std::cout << "Leaving : " << name << std::endl;
  }
 private:
  std::string name;
};


/////////////
// StateTime
// (fires Event UserDEventTimeout on timeout; timer started onEntry)
/////////////
class StateTime : public StateBase {
 public:
 StateTime(const std::string &name_, unsigned milliMaxLifetime_, bool timerRunning_ = true, QState * parent = nullptr)
   : StateBase{name_,            parent}, milliMaxLifetime{milliMaxLifetime_}, timerRunning{timerRunning_}
  {
    setupTimer();
  }
 StateTime(const std::string &name_, unsigned milliMaxLifetime_, ChildMode childMode, bool timerRunning_ = true, QState * parent = nullptr)
   : StateBase{name_, childMode, parent}, milliMaxLifetime{milliMaxLifetime_}, timerRunning{timerRunning_}
  {
    setupTimer();
  }

  void setTimerRunning(bool run) {
    timerRunning = run;
    if (!timerRunning) {
      timer.stop();
    } else {
      if (active()) // setTimerRunning is called on statePing and statePong: only start the timer in the active state!
        timer.start(milliMaxLifetime);
    }
  }
  
 protected:
  void onEntry(QEvent *event) {
    if (timerRunning) {
      switch (static_cast<int>(event->type())) {
      case DEventTimeout:
        {
          const qint64 prevExpiryTimestamp = static_cast<UserDEventTimeout *>(event)->data.millisFromEpoch;
          //std::cout << "prevExpiryTimestamp: " << prevExpiryTimestamp << std::endl;
          timer.startToTimePoint(prevExpiryTimestamp + milliMaxLifetime); // start timer
          //                      ^^ no drift!
        }
        break;
      default:
        timer.start(milliMaxLifetime);
        break;
      }
    }

    StateBase::onEntry(event);
  }

  void onExit(QEvent *event) {

    timer.stop();
    StateBase::onExit(event);
  }

  /*
    private slots:

    void timeout() {
    machine()->postEvent(new UserDEventTimeout{  TimeoutData{timer.expiryTimePoint()} });
    //                                                             ^^ expiry timestamp
    }
  */

 private:
  void setupTimer() {
    timer.setSingleShot(true);

    connect(&timer, &TpTimer::timeout,
            /* fire UserDEventTimeout event */
            [&]() { machine()->postEvent(new UserDEventTimeout{  DEventTimeout, TimeoutData{timer.expiryTimePoint()} }); });
    //                                                                                            ^^ expiry timestamp


    // alternative: use timeout slot above
    // connect(&timer, &TpTimer::timeout, this, &StateTime::timeout);
  }
  
 private:
  unsigned milliMaxLifetime;
  TpTimer timer;
  bool timerRunning;
};




using State = StateTime;


/////////////
// StateMachine
// (with transition table)
/////////////
class StateMachine : public QStateMachine {
 public:
 StateMachine(const std::string &name_, QObject * parent = nullptr)
   : QStateMachine{parent},            name{name_}, timersRunning{true},
     stateTop{this},
     statePing{"statePing", 1000, timersRunning, &stateTop},
     statePong{"statePong", 2000, timersRunning, &stateTop},
     transX_toPing{EventX, &statePong},
     transX_toPong{EventX, &statePing},
     transI{EventI, &stateTop},
     transO{EventO, &stateTop},
     transT{EventT, &stateTop},
     transTimout_toPing{DEventTimeout, always_true<UserDGEventTransition<UserDEventTimeout>::Data>, &statePong}, // see member declaration below for commentary
     transTimout_toPong{DEventTimeout, &statePing}
 {
   initialize_transition_table();
 }

 StateMachine(const std::string &name_, QState::ChildMode childMode, QObject * parent = nullptr)
   : QStateMachine{childMode, parent}, name{name_}, timersRunning{true},
     stateTop{this},
     statePing{"statePing", 1000, timersRunning, &stateTop},
     statePong{"statePong", 2000, timersRunning, &stateTop},
     transX_toPing{EventX, &statePong},
     transX_toPong{EventX, &statePing},
     transI{EventI, &stateTop},
     transO{EventO, &stateTop},
     transT{EventT, &stateTop},
     transTimout_toPing{DEventTimeout, always_true<UserDGEventTransition<UserDEventTimeout>::Data>, &statePong}, // see member declaration below for commentary
     transTimout_toPong{DEventTimeout, &statePing}
 {
   initialize_transition_table();
 }

 void initialize_transition_table()
 {
   setInitialState(&stateTop);
   stateTop.setInitialState(&statePing);
   
   // stateTop  --- transI ----------> statePing
   transI.setTargetState(&statePing);
   stateTop.addTransition(&transI);
   
   // stateTop  --- transO ----------> statePong
   transO.setTargetState(&statePong);
   stateTop.addTransition(&transO);

   

   // statePong --- transX_toPing ---> statePing
   transX_toPing.setTargetState(&statePing);
   statePong.addTransition(&transX_toPing);
   
   // statePing --- transX_toPong ---> statePong
   transX_toPong.setTargetState(&statePong);
   statePing.addTransition(&transX_toPong);


   
   // statePong --- transTimout_toPing ---> statePing
   transTimout_toPing.setTargetState(&statePing);
   statePong.addTransition(&transTimout_toPing);
   
   // statePing --- transTimout_toPong ---> statePong
   transTimout_toPong.setTargetState(&statePong);
   statePing.addTransition(&transTimout_toPong);


   
   // stateTop  --- transT ----------|
   stateTop.addTransition(&transT);
   connect(&transT, &QAbstractTransition::triggered,
           [&]() {
             timersRunning = !timersRunning;
             statePing.setTimerRunning(timersRunning);
             statePong.setTimerRunning(timersRunning);
           });
 }
 
 private:
 std::string name;
 bool timersRunning;
 QState stateTop; // top state, with 2 substates: statePing and statePong
 State statePing;
 State statePong;
 UserEventTransition transX_toPing; // statePong --- transX_toPing ---> statePing
 UserEventTransition transX_toPong; // statePing --- transX_toPong ---> statePong
 UserEventTransition transI;        // stateTop  --- transI ----------> statePing
 UserEventTransition transO;        // stateTop  --- transO ----------> statePong
 UserEventTransition transT;        /* stateTop  --- transT ----------|
                                       this is a targetless transition (http://doc.qt.io/qt-5/statemachine-api.html#targetless-transitions) */


 UserDGEventTransition<UserDEventTimeout> transTimout_toPing; // statePong --- transTimout_toPing ---> statePing

 /* Data guard (DG) actually not used above (we just supply always_true() in constructor. (This is just for demo-purposes)
    Better therefore to use type UserEventTransition. 
    See below for transTimout_toPong!!
 */

 UserEventTransition                      transTimout_toPong; // statePing --- transTimout_toPong ---> statePong
};

#endif
