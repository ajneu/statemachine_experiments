#ifndef USEREVENTTRANSITION_H
#define USEREVENTTRANSITION_H

#include <functional>
#include <QAbstractTransition>
#include "userevents.h"

class QState;

//////////////////////////
//UserEventTransition: Transition without guard (event may carry data, or not)
//////////////////////////
class UserEventTransition : public QAbstractTransition
{
  // Q_OBJECT // Template classes not supported by Q_OBJECT
  
 public:
 UserEventTransition(int eventEnum_, QState * sourceState = nullptr) : QAbstractTransition{sourceState}, eventEnum{eventEnum_} {}
  
 protected:
  virtual bool eventTest(QEvent *e)
  {
    return (e->type() == eventEnum);   // return (e->type() == QEvent::Type(eventEnum));
  }
  
  virtual void onTransition(QEvent *) {}

 public:
  int eventEnum;
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
class UserGEventTransition : public QAbstractTransition {
  // Q_OBJECT // Template classes not supported by Q_OBJECT
 public:
 UserGEventTransition(int eventEnum_, std::function<bool()> guard_ = always_true<>,
                      QState * sourceState = nullptr)
   : QAbstractTransition{sourceState}, eventEnum{eventEnum_}, guard{guard_}
  {
  }

 protected:
  virtual bool eventTest(QEvent *e) {
    if (e->type() != eventEnum) // if (e->type() != QEvent::Type(eventEnum))
      return false;
    return (guard());
  }

  virtual void onTransition(QEvent *) {}

 public:
  int eventEnum;

 private:
  const std::function<bool()> guard;
};


////////////////////////////////////
// UserDGEventTransition: Transition with guard, where guard takes data from event as argument (event must carry data)
////////////////////////////////////
template <typename UDEvent> // UDEvent must be of type UserDataEvent (see userevents.h)
class UserDGEventTransition : public QAbstractTransition {
  // Q_OBJECT // Template classes not supported by Q_OBJECT
 public:
  using Data = typename UDEvent::Data;

 UserDGEventTransition(UserDEventEnum eventEnum_, std::function<bool(typename UDEvent::Data)> guard_ = always_true<typename UDEvent::Data>,
                         QState * sourceState = nullptr)
   : QAbstractTransition{sourceState}, eventEnum{eventEnum_}, guard{guard_}
  {
  }

 protected:
  virtual bool eventTest(QEvent *e) {
    if (e->type() != QEvent::Type(eventEnum))
      return false;
    UDEvent *ude = static_cast<UDEvent*>(e);
    return (guard(ude->data));
  }

  virtual void onTransition(QEvent *) {}

 public:
  UserDEventEnum eventEnum;

 private:
  const std::function<bool(typename UDEvent::Data)> guard;
};










/*
///// EXAMPLES
// user event that does not carry data
UserEvent x{EventX};
UserEventTransition tr{EventX};

// user event carrying data (an int)

UserDataEvent<int> y(EventI, 3);
UserDGEventTransition<UserDataEvent<int>> tr2(EventI, [](int){return true;});

QState s;
UserDGEventTransition<UserDataEvent<int>> *tr3 = new UserDGEventTransition<UserDataEvent<int>>(EventI, always_true<typename UserDataEvent<int>::Data>, &s);
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
