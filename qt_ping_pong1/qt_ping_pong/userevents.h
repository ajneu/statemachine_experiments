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


struct UserEvent : public QEvent
{
 UserEvent(UserEventEnum eventEnum_ = UserEventEnum(QEvent::User)) : QEvent(QEvent::Type(eventEnum_)) {}
 UserEvent(const UserEvent &other) : QEvent(other.type()) {}
};


template <typename Data_>
struct UserDataEvent : public QEvent
{
  using Data = Data_;

 UserDataEvent(UserDEventEnum eventEnum_ = UserDEventEnum(QEvent::User)) : QEvent(QEvent::Type(eventEnum_)),  data{} {}
 UserDataEvent(UserDEventEnum eventEnum_, const Data_ &data_)            : QEvent(QEvent::Type(eventEnum_)),  data(data_) {}
 UserDataEvent(const UserDataEvent<Data_> &other)                        : QEvent(other.type()),              data(other.data) {}

  Data_ data;
};


// Data for DEventTimeout
struct TimeoutData {
TimeoutData(qint64 millis=0) : millisFromEpoch{millis} {}
  qint64 millisFromEpoch;
};


// Type aliases
using UserDEventTimeout = UserDataEvent<TimeoutData>;


Q_DECLARE_METATYPE(UserEvent)
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
