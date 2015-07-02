#include <iostream>
#include <QCoreApplication>
#include <QTimer>
#include "interfacethread.h"

#include "statemachine.h"
#include "interlayer_connections.h"

int main(int argc, char *argv[])
{
  QCoreApplication app{argc, argv};

  register_metatype_userevents();


  std::cout <<
    "There are 2 states: statePing and statePong\n"
    "When timer running then:\n"
    "Maximum lifetime of statePing is 1000 ms - it will then automatically transition to statePong;\n"
    "Maximum lifetime of statePong is 2000 ms - it will then automatically transition to statePing.\n"
    "\n"
    "Keyboard-Input can cause transitions before the max-lifetime-timeouts:\n"
    "'x': xchange state\n"
    "'i': leave current state and go to statePing (pIng)\n"
    "'o': leave current state and go to statePong (pOng)\n"
    "'t': toggle timer (on/off)\n"
    "'q' or eof (Ctrl-d): exit\n"
    "\n"
    "...Hit Enter to start!" << std::flush;

  std::cin.ignore();
  
  // thread that handles user's keyboard input
  InterfaceThread th;
  th.start();
  publish_interface_to_interlayer(th);

  // statemachine (running in eventloop)
  StateMachine sm{"statemachine"};
  subscribe_statemachine_to_interlayer(sm);
  sm.start();

  return app.exec();
}
