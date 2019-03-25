// This example demonstrates how a class based actor
// can monitor other actor(s)
//
//
// An actor can get killed and/or declares itself dead. There are use cases
// where other actors may be interested in knowing about such events.
//
// Here we will build an actor that adds to its state what is supplied in its
// message until it reaches 3. If the new state >= 3 then
// it would quit (i.e. terminate itself)
//
// Please note that quit() means that it sort of logically unregisters itself
// from the actor system however the object still remains alive until all the
// references to it go away (explicity or implicitly).

#include <iostream>
#include <string>

#include <caf/all.hpp>

using add_atom = caf::atom_constant<caf::atom("add")>;

// this is a trait for actors that will add until
// they reach 3 and then they should terminate themselves
using uptill_3_adder = caf::typed_actor<caf::reacts_to<add_atom, int>>;

class AddUntilReach3 : public uptill_3_adder::base {
public:
  AddUntilReach3(caf::actor_config &cfg) : uptill_3_adder::base(cfg) {
    this->_value = 0;
  }
  ~AddUntilReach3() {
    std::cout << "[CLS] AddUntilReach3 Destructor called" << std::endl;
  }

public:
  AddUntilReach3::behavior_type make_behavior() {
    return {

        // the behaviors for this actor
        [=](add_atom, int x) {
          caf::aout(this) << "[CLS] Current Value is " << this->_value
                          << std::endl;
          this->_value += x;
          caf::aout(this) << "[CLS] New Value is " << this->_value << std::endl;

          if (this->_value >= 3) {
            caf::aout(this)
                << "[CLS] Actor reached its purpose so it will terminate "
                << std::endl;

            // You can invoke quit with out any error code
            // as well
            this->quit(caf::exit_reason::user_shutdown);
          }
        }};
  }

private:
  // state is maintained in the class based actor
  int _value;
};

class ASpawnerActor : public caf::event_based_actor {
public:
  ASpawnerActor(caf::actor_config &cfg)
      : caf::event_based_actor(cfg),
        _adder(std::move(this->spawn<AddUntilReach3, caf::monitored>())) {

    // Pay attention to how it was specified that this actor (i.e.
    // ASpawnerActor) would like to monitor AddUntilReach3
    //
    // Note that this is not the only way to specify the intention to
    // monitor. You could simply do this->monitor(other_actor) as well
    // where other_actor the reference/handle of the actor to monitor

    // Another important piece of code is how you would specify
    // the lambda that would get invoked on quit events
    set_down_handler([=](caf::down_msg &msg) {
      std::cout << "Received a down message" << std::endl;

      // this is one way to know which actor was terminated
      if (msg.source == _adder.address()) {
        std::cout << "We received down message for adder actor " << std::endl;
      }
    });
  }

  ~ASpawnerActor() {
    std::cout << "[CLS] ASpawnerActor Destructor called" << std::endl;
  }

  void on_exit() override {
    // This is a special method and gives you an opportunity to
    // destroy (clean up) any references (especially that of other actors)
    std::cout << "in on_exit of ASpawnerActor " << std::endl;
    destroy(_adder);
  }

  caf::behavior make_behavior() override {
    return {[=](int x, int y) {
      // triggers 3 adds
      this->send(this->_adder, add_atom::value, 1);
      this->send(this->_adder, add_atom::value, 1);
      this->send(this->_adder, add_atom::value, 1);

      return x + y;
    }};
  }

private:
  uptill_3_adder _adder;
};

void caf_main(caf::actor_system &system) {
  caf::scoped_actor self{system};

  auto spawner = self->spawn<ASpawnerActor>();

  self->send(spawner, 2, 1);

  std::cout << "Press any char to terminate .." << std::endl;
  std::cin.get();
}

CAF_MAIN()