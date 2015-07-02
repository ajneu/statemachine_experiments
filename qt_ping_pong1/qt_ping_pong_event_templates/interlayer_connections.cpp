#include "interlayer_connections.h"
#include "interlayer.h"

#include <functional>
#include <QObject>

#include <iostream>

/*
namespace {

  template <typename Event>
  void postToStateMachine(const Event &e, QStateMachine &sm)
  {
    sm.postEvent(new Event{e});
  }
  
}
*/

void publish_interface_to_interlayer(InterfaceThread &th)
{
  QObject::connect(&th, &InterfaceThread::signalEventX, &interlayer, &Interlayer::signalEventX /*, Qt::QueuedConnection */);
  QObject::connect(&th, &InterfaceThread::signalEventI, &interlayer, &Interlayer::signalEventI /*, Qt::QueuedConnection */);
  QObject::connect(&th, &InterfaceThread::signalEventO, &interlayer, &Interlayer::signalEventO /*, Qt::QueuedConnection */);
  QObject::connect(&th, &InterfaceThread::signalEventT, &interlayer, &Interlayer::signalEventT /*, Qt::QueuedConnection */);
}


void subscribe_statemachine_to_interlayer(StateMachine &sm)
{
  QObject::connect(&interlayer, &Interlayer::signalEventX,
                   //std::bind(&(postToStateMachine<UserEventX>), std::placeholders::_1, std::ref(sm)));
                   [&](const UserEventX &e) { sm.postEvent(new UserEventX{e});
                     std::cout << "posting EventX" << std::endl;});

  QObject::connect(&interlayer, &Interlayer::signalEventO,
                   //std::bind(&(postToStateMachine<UserEventO>), std::placeholders::_1, std::ref(sm)));
                   [&](const UserEventO &e) { sm.postEvent(new UserEventO{e});
                     std::cout << "posting EventO" << std::endl;});

  QObject::connect(&interlayer, &Interlayer::signalEventI,
                   //std::bind(&(postToStateMachine<UserEventI>), std::placeholders::_1, std::ref(sm)));
                   [&](const UserEventI &e) { sm.postEvent(new UserEventI{e});
                     std::cout << "posting EventI" << std::endl; });

  QObject::connect(&interlayer, &Interlayer::signalEventT,
                   //std::bind(&(postToStateMachine<UserEventT>), std::placeholders::_1, std::ref(sm)));
                   [&](const UserEventT &e) { sm.postEvent(new UserEventT{e});
                     std::cout << "posting EventT" << std::endl; });

}
