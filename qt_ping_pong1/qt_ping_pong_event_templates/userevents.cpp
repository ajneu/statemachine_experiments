#include "userevents.h"
#include <QMetaType>

bool isEventType(QEvent::Type eventEnum, const QEvent *event)
{
  return (event->type() == eventEnum);
}

void register_metatype_userevents()
{
  qRegisterMetaType<UserEventX>("UserEventX");
  qRegisterMetaType<UserEventI>("UserEventI");
  qRegisterMetaType<UserEventO>("UserEventO");
  qRegisterMetaType<UserEventT>("UserEventT");
  qRegisterMetaType<UserDEventTimeout>("UserDEventTimeout");
}
