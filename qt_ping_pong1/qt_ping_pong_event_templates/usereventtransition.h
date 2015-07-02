#ifndef USEREVENTTRANSITION_H
#define USEREVENTTRANSITION_H

#include <functional>
#include <QAbstractTransition>
#include "userevents.h"

class QState;

//////////////////////////
//UserEventTransition: Transition without guard (event may carry data, or not)
//////////////////////////
template <typename UEvent> // UEvent can be of type UserEvent or UserDEventEnum (see userevents.h)
class UserEventTransition : public QAbstractTransition
{
  // Q_OBJECT // Template classes not supported by Q_OBJECT
  
 public:
 UserEventTransition(QState * sourceState = nullptr) : QAbstractTransition{sourceState} {}
  
 protected:
  virtual bool eventTest(QEvent *e)
  {
    return (e->type() == QEvent::Type(UEvent::eventEnum));
  }
  
  virtual void onTransition(QEvent *) {}
};


// default argument for guard-condition below
template <typename... T>
constexpr bool always_true(T...)
{
  return true;
}



////////////////////////////////////
// UserGEventTransition: Transition with guard, but guard does not take data from event (therefore: event may carry data, or not)
////////////////////////////////////
template <typename UEvent> // UEvent can be of type UserEvent or UserDEventEnum (see userevents.h)
class UserGEventTransition : public QAbstractTransition
{
  // Q_OBJECT // Template classes not supported by Q_OBJECT
  
 public:
 UserGEventTransition(std::function<bool()> guard_ = always_true<>, QState * sourceState = nullptr)
   : QAbstractTransition{sourceState}, guard{guard_}
  {
  }

 protected:
  virtual bool eventTest(QEvent *e)
  {
    if (e->type() != QEvent::Type(UEvent::eventEnum))
      return false;
    return (guard());
  }
  
  virtual void onTransition(QEvent *) {}

 private:
  const std::function<bool()> guard;
};


// default argument for guard-condition below
template <typename T>
constexpr bool always_true(T)
{
  return true;
}



////////////////////////////////////
// UserDGEventTransition: Transition with guard, where guard takes data from event as argument (event must carry data)
////////////////////////////////////
template <typename UDEvent> // UDEvent must be of type UserDataEvent (see userevents.h)
class UserDGEventTransition : public QAbstractTransition {
  // Q_OBJECT // Template classes not supported by Q_OBJECT
 public:
  using Data = typename UDEvent::Data;

 UserDGEventTransition(std::function<bool(typename UDEvent::Data)> guard_ = always_true<typename UDEvent::Data>,
                         QState * sourceState = nullptr)
   : QAbstractTransition{sourceState}, guard{guard_}
  {
  }

 protected:
  virtual bool eventTest(QEvent *e) {
    if (e->type() != QEvent::Type(UDEvent::eventEnum))
      return false;
    UDEvent *ude = static_cast<UDEvent*>(e);
    return (guard(ude->data));
  }

  virtual void onTransition(QEvent *) {}

 private:
  const std::function<bool(typename UDEvent::Data)> guard;
};









/*
///// EXAMPLES
// user event that does not carry data
UserEvent<EventX> x;
UserEventTransition<UserEvent<EventX>> tr;

// user event carrying data (an int)

UserDataEvent<EventI, int> y(3);
UserDGEventTransition<UserDataEvent<EventI, int>> tr2([](int){return true;});

QState s;
UserDGEventTransition<UserDataEvent<EventI, int>> *tr3 = new UserDGEventTransition<UserDataEvent<EventI, int>>(always_true<typename UserDataEvent<EventI, int>::Data>, &s);
*/


/*
http://eli.thegreenplace.net/2014/variadic-templates-in-c/
template <class... Ts> struct tuple {};

template <class T, class... Ts>
  struct tuple<T, Ts...> : tuple<Ts...> {
 tuple(T t, Ts... ts) : tuple<Ts...>(ts...), tail(t) {}

  T tail;
};
*/

/*
template <typename... Targs>
class Transition {
public:
  Transition(std::function<bool(Targs...)> pred_ = &always_true) : pred{pred_} {}
  bool check(Targs... Fargs) { return pred(Fargs...); }
private:
  std::function<bool(Targs...)> pred;
};
*/

#endif
