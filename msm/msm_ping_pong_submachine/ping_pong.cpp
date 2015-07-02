#include <iostream>
#include <string>
#include <cctype>

#include <chrono>
#include <thread>
#include <functional>

#include <experimental/optional>

#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/back/state_machine.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/signals2.hpp>


namespace msm = boost::msm;
namespace mpl = boost::mpl;



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




// base to give states names
struct NameBase {
  NameBase(const std::string& name_) : name{name_} {}
  
  const std::string& get_name() const { return name; }
  
private:
  const std::string name;
};

// base to print on_entry and on_exit messages
struct StateBase : public msm::front::state<>, public NameBase
{
  StateBase(const std::string& name_ = "StateBase_DefaultName") : NameBase{name_} { std::cout << "instantiating object " << get_name() << std::endl; }

  template <class Event, class FSM>
  void on_entry(const Event&, FSM&) {std::cout << "Entering: " << get_name() << std::endl;}
  
  template <class Event, class FSM>
  void on_exit(const Event&,FSM&)   {std::cout << "Leaving : " << get_name() << std::endl;}
};



///////// Machine Base - VERION 0
// struct StateTop_ : public msm::front::state_machine_def<StateTop_, StateBase>
// {
// public:
//   StateTop_(const std::string& name_) {}



///////// Machine Base - VERION 1
template <typename Machine>
struct StateMachineBase : public msm::front::state_machine_def<StateMachineBase<Machine>>, public NameBase
{
public:
  StateMachineBase(const std::string& name_) : NameBase{name_} {}

  template <class Event, class FSM>
  void on_entry(const Event&, FSM&) {std::cout << "Entering: " << get_name() << std::endl;}
  
  template <class Event, class FSM>
  void on_exit(const Event&,FSM&)   {std::cout << "Leaving : " << get_name() << std::endl;}

};

struct StateMachine_ : public StateMachineBase<StateMachine_>
{
  StateMachine_(const std::string& name_) : StateMachineBase{name_} {}
  
  // front-end: define StateTop as Submachine
  struct StateTop_ : public StateMachineBase<StateTop_>
  {
  public:
    StateTop_(const std::string& name_ = "StateTop") : StateMachineBase{name_} {}
    
    ////////////
    // StatePing
    ////////////
    struct StatePing : StateBase {
      StatePing() : StateBase("StatePing") {}
      
      template <class Event, class FSM>
      void on_entry(const Event& event, FSM& fsm) { StateBase::on_entry(event, fsm); /*fsm.process_event(EventX{});*/ }
    };
    
    ////////////
    // StatePong
    ////////////
    struct StatePong : StateBase {
      StatePong() : StateBase("StatePong") {}
      
      template <class Event, class FSM>
      void on_entry(const Event& event, FSM& fsm) { StateBase::on_entry(event, fsm); /*fsm.process_event(EventX{});*/ }
    };
    
    typedef StatePing initial_state;
    
    struct transition_table : mpl::vector<
      _row<StatePing, EventX, StatePong>,
      _row<StatePong, EventX, StatePing>
      >{};
  };
  
  typedef msm::back::state_machine<StateTop_> StateTop;

  typedef StateTop initial_state;

  struct transition_table : mpl::vector<
    _row<StateTop, EventO, StateTop::StatePong>,
    
    _row<StateTop, EventI, StateTop::StatePing>
    >{};

};

typedef msm::back::state_machine<StateMachine_> StateMachine;

/*
int main()
{
  StateMachine sm("StateMachine");
  sm.start();

  for (char c; std::cin >> c; ) {
    switch (tolower(c)) {
    case 'x':
      sm.process_event(EventX{});
      break;
    case 'q':
      goto label_stop;
      break;
    }
  }

 label_stop:
  sm.stop();
  return 0;
}
*/








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

  StateMachine sm{"StateMachine"}; //, io_service};
  sm.start();
  
  interface.connect([&](EventID eid) {
      switch (eid) {
      case eidI:
        io_service.post(std::bind(static_cast<boost::msm::back::execute_return (StateMachine::*)(const EventI &event)>(&StateMachine::process_event),
                                  &sm, EventI{})); // go to state ping
        //sm.process_event(EventI{}); 
        break;
      case eidO:
        io_service.post(std::bind(static_cast<boost::msm::back::execute_return (StateMachine::*)(const EventO &event)>(&StateMachine::process_event),
                                  &sm, EventO{})); // go to state pong
        //sm.process_event(EventO{});
        break;
      case eidX:
        io_service.post(std::bind(static_cast<boost::msm::back::execute_return (StateMachine::*)(const EventX &event)>(&StateMachine::process_event),
                                  &sm, EventX{})); // xchange state
        //sm.process_event(EventX{});
        break;
      case eidT:
        io_service.post(std::bind(static_cast<boost::msm::back::execute_return (StateMachine::*)(const EventT &event)>(&StateMachine::process_event),
                                  &sm, EventT{})); // toggle timer on/off
        //sm.process_event(EventT{});
        break;
      case eidQ:
        std::cout << "in quit" << std::endl;
        io_service.post(std::bind(static_cast<void (StateMachine::*)()>(&StateMachine::stop), &sm)); // stop machine
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
