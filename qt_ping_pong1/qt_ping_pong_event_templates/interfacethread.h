#ifndef INTERFACETHREAD_H
#define INTERFACETHREAD_H

#include <iostream>
#include <cctype>
#include <QThread>
#include <QObject>
#include <QCoreApplication> /* qApp */

#include "interlayer.h"

class InterfaceThread : public QThread
{
  Q_OBJECT
  void run() {

    for (char c; std::cin >> c; ) {
      switch (tolower(c)) {
      case 'x':
        std::cout << "xchange" << std::endl;
        emit signalEventX(UserEventX{});

        //emit interlayer.signalEventX(UserEventX{});
        /* Above is only safe if thread affinity of interlayer is this thread
           Requires this in main: interlayer.moveToThread(&th); // th is instance of InterfaceThread
           subscribe_statemachine_to_interlayer() must come thereafter and its connect calls will then cause get Qt::QueuedConnection
        */
        
        break;
      case 'o':
        std::cout << "pong" << std::endl;
        emit signalEventO(UserEventO{});
        //emit interlayer.signalEventO(UserEventO{});
        break;
      case 'i':
        std::cout << "ping" << std::endl;
        emit signalEventI(UserEventI{});
        //emit interlayer.signalEventI(UserEventI{});
        break;
      case 't':
        std::cout << "toggle timers" << std::endl;
        emit signalEventT(UserEventT{});
        break;
      case 'q':
        goto label_stop;
        break;
      }
    }
  label_stop:
    qApp->quit();
  }

 signals:
  void signalEventX(const UserEventX &);
  void signalEventO(const UserEventO &);
  void signalEventI(const UserEventI &);
  void signalEventT(const UserEventT &);
};


#endif
