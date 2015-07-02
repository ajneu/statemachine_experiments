#ifndef SUBSCRIBEINTERLAYER_H
#define SUBSCRIBEINTERLAYER_H

#include "statemachine.h"
#include "interfacethread.h"

void subscribe_statemachine_to_interlayer(StateMachine &sm);
void publish_interface_to_interlayer(InterfaceThread &th);

#endif
