#include "userevents.h"
#include <QMetaType>

bool isEventType(QEvent::Type eventEnum, const QEvent *event)
{
  return (event->type() == eventEnum);
}

void register_metatype_userevents()
{
  qRegisterMetaType<UserEvent>("UserEvent");
  qRegisterMetaType<UserDEventTimeout>("UserDEventTimeout");
}
