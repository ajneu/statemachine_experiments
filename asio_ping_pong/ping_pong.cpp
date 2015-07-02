#include <iostream>
#include <string>
#include <cctype>

#include <chrono>
#include <thread>
#include <functional>

#include <experimental/optional>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/signals2.hpp>




////////////////////////
// base-class for states
// print message for entry or exit to state
////////////////////////
class StateBase {
public:
  StateBase(const std::string& name_) : name{name_} {}

  virtual ~StateBase() {} // polymorphic
  
  template <typename Event, typename FSM>
  void on_entry(const Event&, FSM&) { std::cout << "Entering: " << name << std::endl; }

  template <typename Event, typename FSM>
  void on_exit(const Event&, FSM&) { std::cout << "Leaving : " << name << std::endl; }

private:
  std::string name;
  
};



// Data for DEventTimeout
struct TimeoutData {
  std::chrono::steady_clock::time_point time_point;
};

//////////
// events
//////////
struct EventI {};  // pIng    event: leave current state and go to ping state
struct EventO {};  // pOng    event: leave current state and go to pong state
struct EventX {};  // xchange event: change between ping and pong
struct EventT {};  // toggle timer on/off
struct DEventTimeout {
  TimeoutData data;  /* Timeout Event
                        This will be a DataEvent [DEvent] carrying the timestamp-of-timeout.
                        Reason:
                        if we timeout and enter a new state; and setup a new timer, there is a brief delay until that timer is running.
                        This could cause timer drift.
                        Therefore the timeout event carries the timestamp-of-timeout, so that the new timer can be
                        setup (taking into consideration timestamp-of-timeout), leading to *no* timer drift!
                     */
};


enum EventID {
  eidI, // pIng
  eidO, // pOng
  eidX, // xchange
  eidT, // toggle timer on/off
  eidQ  // quit
};


////////////////
// State Machine
////////////////
class StateMachine : public StateBase {
public:
  StateMachine(const std::string& name_, boost::asio::io_service &io_service_) :
    StateBase{name_}, timer_running{true},
    statePing{"statePing", std::chrono::milliseconds(1000), io_service_, timer_running},
    statePong{"statePong", std::chrono::milliseconds(2000), io_service_, timer_running},
    current_state{&statePing} {}

  void start()
  {
    // enter initial state
    if (StatePing* p_ping = dynamic_cast<StatePing*>(current_state)) {
      p_ping->on_entry(0, *this);
    } else if (StatePong* p_pong = dynamic_cast<StatePong*>(current_state)) {
      p_pong->on_entry(0, *this);
    }    
  }

  void stop()
  {
    leave_state(0);
  }

  /*
    .       EventI
    state ----------> statePing
  */
  void process_event(const EventI &event) {
    change_to_state(event, statePing);
  }

  /*
    .       EventO
    state ----------> statePong
  */
  void process_event(const EventO &event) {
    change_to_state(event, statePong);
  }


  /*
    .           EventX
    statePing ----------> statePong

    .           EventX
    statePong ----------> statePing
  */
  void process_event(const EventX &event) {
    if (current_state == &statePing) {
      change_to_state(event, statePong);
    } else /* if (current_state == &statePong) */ {
      change_to_state(event, statePing);
    }
  }

  /*
    .           DEventTimeout
    statePing -----------------> statePong

    .           DEventTimeout
    statePong -----------------> statePing

  */
  void process_event(const DEventTimeout &event) {
    if (current_state == &statePing) {
      change_to_state(event, statePong);
    } else /* if (current_state == &statePong) */ {
      change_to_state(event, statePing);
    }
  }

  /*
    .       EventT
    state ----------| 
  */
  void process_event(const EventT &event) {
    timer_running = !timer_running;
    statePing.set_timer_running(timer_running, *this, current_state);
    statePong.set_timer_running(timer_running, *this, current_state);
    /*
      if (StatePing* p_ping = dynamic_cast<StatePing*>(current_state)) {
        p_ping->on_entry(event, *this);
      } else if (StatePong* p_pong = dynamic_cast<StatePong*>(current_state)) {
        p_pong->on_entry(event, *this);
      }
    */
  }

  
private:
  
  // ##### StateTime #####
  struct StateTime : public StateBase {
    StateTime(const std::string &name, std::chrono::milliseconds max_lifetime_, boost::asio::io_service &io_service_, bool timer_running_)
      : StateBase{name}, max_lifetime{max_lifetime_}, timer{io_service_}, timer_running{timer_running_} {}

    template <typename Event, typename FSM> // see overloads below
    void on_entry(const Event &event, FSM &fsm) {
      if (timer_running) {
        timer.expires_from_now(max_lifetime);
        start_timer(fsm);
      }
      StateBase::on_entry(event, fsm);
    }

    template <typename FSM> // overload: specializing Event to DEventTimeout
    void on_entry(const DEventTimeout &event, FSM &fsm) {
      if (timer_running) {
        timer.expires_at(event.data.time_point + max_lifetime);
        start_timer(fsm);
      }
      StateBase::on_entry(event, fsm);
    }

    template <typename FSM> // overload: specializing Event to EventT (toggle timer) -- this is currently not called (see set_timer_running() below)
    void on_entry(const EventT &event, FSM &fsm) {
      if (timer_running) {
        timer.expires_from_now(max_lifetime);
        start_timer(fsm);
      } else {
        timer.cancel();
      }
      // StateBase::on_entry(event, fsm); // don't call this line, or we would print entry to a state, in which we are already in
    }
    
    template <typename Event, typename FSM>
    void on_exit(const Event &event, FSM &fsm) {
      timer.cancel();
      StateBase::on_exit(event, fsm);
    }

    template <typename FSM>
    void set_timer_running(bool run, FSM &fsm, StateBase *current_state) {
      timer_running = run;
      if (timer_running) {
        if (current_state == this) {
          /* because of the following, we don't send EventT into the state itself
             (see overload specializing Event to EventT)
          */
          timer.expires_from_now(max_lifetime);
          start_timer(fsm);
        }
      } else {
        timer.cancel();
      }
    }
    
  private:
    template <typename FSM>
    void timeout(const boost::system::system_error &err, FSM &fsm) {
      if (err.code() == boost::system::errc::success)
        fsm.process_event(DEventTimeout{{timer.expires_at()}});
    }

    template <typename FSM>
    void start_timer(FSM &fsm) {
        timer.async_wait(std::bind(&StateTime::timeout<FSM>, this, std::placeholders::_1, std::ref(fsm)));
    }
    
  private:
    std::chrono::milliseconds max_lifetime;
    boost::asio::steady_timer timer;
    bool timer_running;
  };

  // ##### StatePing #####
  struct StatePing : public StateTime {
    using StateTime::StateTime;
  };

  // ##### StatePong #####
  struct StatePong : public StateTime {
    using StateTime::StateTime;
  };


  template <typename Event>
  void leave_state(const Event& event) {
    // leave old state
    if (StatePing* p_ping = dynamic_cast<StatePing*>(current_state)) {
      p_ping->on_exit(event, *this);
    } else if (StatePong* p_pong = dynamic_cast<StatePong*>(current_state)) {
      p_pong->on_exit(event, *this);
    }
  }

  template <typename Event, typename State>
  void change_to_state(const Event& event, State &newState) {
    leave_state(event);
    
    // set   new state
    current_state = &newState;  
    newState.on_entry(event, *this);
  }


  bool timer_running;
  StatePing statePing;
  StatePong statePong;
  
  StateBase *current_state;
  
};


// class Interface {
// public:

//   void run_statemachine(boost::asio::io_service &io_service) {
//     for (char c; std::cin >> c; ) {
//       switch (tolower(c)) {
//       case 'i':
//         //sm.process_event(EventI{}); // go to state ping
//         break;
//       case 'o':
//         //sm.process_event(EventO{}); // go to state pong
//         break;
//       case 'x':
//         //sm.process_event(EventX{}); // xchange state
//         break;
//       case 'q':
//         goto label_stop;
//         break;
//       }
//     }
//   label_stop:
//     ;
//   }
// };


class Interface {
  using T = EventID;
public:
  Interface(boost::asio::io_service &io_service_) : io_service{io_service_} {}
  
  void run_statemachine() {
    for (char c; std::cin >> c; ) {
      switch (tolower(c)) {
      case 'i':
        //sm.process_event(EventI{}); // go to state ping
        sig(eidI);
        break;
      case 'o':
        //sm.process_event(EventO{}); // go to state pong
        sig(eidO);
        break;
      case 'x':
        //sm.process_event(EventX{}); // xchange state
        sig(eidX);
        break;
      case 't':
        //sm.process_event(EventT{}); // toggle timer on/off
        sig(eidT);
        break;
      case 'q':
        goto label_stop;
        break;
      }
    }
  label_stop:
    sig(eidQ);
  }

  void connect(std::function<void(T)> f) {
    sig.connect(f);
  }
  
  
  private:
  boost::signals2::signal<void(T)> sig;
  boost::asio::io_service &io_service;
};


int main()
{
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


  boost::asio::io_service io_service;

  //work to keep io_service busy
  std::experimental::optional<boost::asio::io_service::work> work(std::experimental::in_place, io_service);
  /* https://think-async.com/Asio/TipsAndTricks#Stopping_the_io_service_from_run */

  
  Interface interface{io_service};

  /*
  Interface interface;
  std::thread th(&Interface::run_statemachine, &interface, std::ref(io_service));
  */

  
  std::thread th(&Interface::run_statemachine, &interface);

  StateMachine sm{"StateMachine", io_service};
  sm.start();
  
  interface.connect([&](EventID eid) {
      switch (eid) {
      case eidI:
        io_service.post(std::bind(static_cast<void (StateMachine::*)(const EventI &event)>(&StateMachine::process_event),
                                  &sm, EventI{})); // go to state ping
        //sm.process_event(EventI{}); 
        break;
      case eidO:
        io_service.post(std::bind(static_cast<void (StateMachine::*)(const EventO &event)>(&StateMachine::process_event),
                                  &sm, EventO{})); // go to state pong
        //sm.process_event(EventO{});
        break;
      case eidX:
        io_service.post(std::bind(static_cast<void (StateMachine::*)(const EventX &event)>(&StateMachine::process_event),
                                  &sm, EventX{})); // xchange state
        //sm.process_event(EventX{});
        break;
      case eidT:
        io_service.post(std::bind(static_cast<void (StateMachine::*)(const EventT &event)>(&StateMachine::process_event),
                                  &sm, EventT{})); // toggle timer on/off
        //sm.process_event(EventT{});
        break;
      case eidQ:
        io_service.post(std::bind(&StateMachine::stop, &sm)); // stop machine
        work = std::experimental::nullopt; /* https://think-async.com/Asio/TipsAndTricks#Stopping_the_io_service_from_run */
        break;
      default:
        break;
      }
    }
    );
  
  io_service.run();
  
  th.join();
  
  return 0;
}
