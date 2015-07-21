#include "interlayer_connections.h"
#include "interlayer.h"

#include <functional>
#include <QObject>

#include <iostream>

void publish_interface_to_interlayer(InterfaceThread &th)
{
  QObject::connect(&th, &InterfaceThread::signalEvent,     &interlayer, &Interlayer::signalEvent     /*, Qt::QueuedConnection */);
  QObject::connect(&th, &InterfaceThread::signalDataEvent, &interlayer, &Interlayer::signalDataEvent /*, Qt::QueuedConnection */);
}


void subscribe_statemachine_to_interlayer(StateMachine &sm)
{
  QObject::connect(&interlayer, &Interlayer::signalEvent,
                   [&](const UserEvent &e) {
                     bool post{false};
                     switch (static_cast<int>(e.type())) {
                     case EventX:
                       post = true;
                       std::cout << "posting EventX" << std::endl;
                       break;
                     case EventO:
                       post = true;
                       std::cout << "posting EventO" << std::endl;
                       break;
                     case EventI:
                       post = true;
                       std::cout << "posting EventI" << std::endl;
                       break;
                     case EventT:
                       post = true;
                       std::cout << "posting EventT" << std::endl;
                       break;
                     default:
                       break;
                     }

                     if (post) {
                       sm.postEvent(new UserEvent{e});
                     }
                   });
}
