#ifndef USEREVENTS_H
#define USEREVENTS_H

#include <QEvent>
#include <QMetaType>

enum UserEventEnum{
  EventX = QEvent::User,   // xchange
  EventI,                  // pIng
  EventO,                  // pOng
  EventT,                  // Toggle
  DEvent_Num               // enum below will start with this value
};

enum UserDEventEnum{
  DEventTimeout = DEvent_Num /* Timeout Event
                                This will be a DataEvent [DEvent] carrying the timestamp-of-timeout.
                                Reason: 
                                if we timeout and enter a new state; and setup a new timer, there is a brief delay until that timer is running.
                                This could cause timer drift.
                                Therefore the timeout event carries the timestamp-of-timeout, so that the new timer can be
                                setup (taking into consideration timestamp-of-timeout), leading to *no* timer drift!
                             */
};

// see using type aliases at the bottom



template <UserEventEnum eventEnum_>
struct UserEvent : public QEvent
{
  static constexpr UserEventEnum eventEnum = eventEnum_;
 UserEvent() : QEvent(QEvent::Type(eventEnum_)) {}

 UserEvent(const UserEvent<eventEnum_> &/*other*/) : UserEvent() {}
};


template <UserDEventEnum eventEnum_, typename Data_>
struct UserDataEvent : public QEvent
{
  static constexpr UserDEventEnum eventEnum = eventEnum_;
  using Data = Data_;
  
 UserDataEvent()                   : QEvent(QEvent::Type(eventEnum_)), data{0} {}
 UserDataEvent(const Data_ &data_) : QEvent(QEvent::Type(eventEnum_)), data(data_) {}
 UserDataEvent(const UserDataEvent<eventEnum_, Data_> &other) : UserDataEvent(other.data) {}
  
  Data_ data;
};


// Data for DEventTimeout
struct TimeoutData {
  qint64 millisFromEpoch;
};


// Type aliases
using UserEventX        = UserEvent<    EventX>;                 // xchange
using UserEventI        = UserEvent<    EventI>;                 // pIng
using UserEventO        = UserEvent<    EventO>;                 // pOng
using UserEventT        = UserEvent<    EventT>;                 // Toggle
using UserDEventTimeout = UserDataEvent<DEventTimeout, TimeoutData>;


Q_DECLARE_METATYPE(UserEventX)
Q_DECLARE_METATYPE(UserEventI)
Q_DECLARE_METATYPE(UserEventO)
Q_DECLARE_METATYPE(UserDEventTimeout)

void register_metatype_userevents();

/*
// don't use this template-function: it would increase the generated code-size. See below for normal function-call
template <QEvent::Type eventEnum>
bool isEventType(const QEvent *event)
{
  return (event->type() == eventEnum);
}
*/

bool isEventType(QEvent::Type eventEnum, const QEvent *event);



/* QVariant example:


  QVariant v;

  v = 1;
  v = 1.23;

  UserEventX x;
  v.setValue(x);
  UserEventX xx = v.value<UserEventX>();

*/

#endif
