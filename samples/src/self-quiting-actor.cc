#include <iostream>
#include <string>

#include "caf/all.hpp"

using add_atom = caf::atom_constant<caf::atom("add")>;

// this is a trait for actors that will add until
// they reach 3 and then they should terminate themselves
using uptill_3_adder = caf::typed_actor<caf::reacts_to<add_atom, int>>;

// a class based actor
class AddUntilReach3 : public uptill_3_adder::base {
public:
  AddUntilReach3(caf::actor_config &cfg) : uptill_3_adder::base(cfg) {
    this->_value = 0;
  }
  ~AddUntilReach3() { std::cout << "[CLS] Destructor called" << std::endl; }

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
            this->quit();
          }
        }};
  }

private:
  // state is maintained in the class based actor
  int _value;
};

// State for the function based actor
struct StateForFunctionBasedActor {
  int value;

  ~StateForFunctionBasedActor() {
    std::cout << "StateForFunctionBasedActor is being destroyed" << std::endl;
  }
};

// a function based actor
uptill_3_adder::behavior_type add_until_reach3(
    uptill_3_adder::stateful_pointer<StateForFunctionBasedActor> self) {

  return {// the behaviors for this actor
          [=](add_atom, int x) {
            caf::aout(self)
                << "[Fun] Current Value is " << self->state.value << std::endl;
            self->state.value += x;
            caf::aout(self)
                << "[Fun] New Value is " << self->state.value << std::endl;

            if (self->state.value >= 3) {
              caf::aout(self)
                  << "[Fun] Actor reached its purpose so it will terminate "
                  << std::endl;
              self->quit();
            }
          }};
}

void test_cls_actor(caf::actor_system &system) {
  caf::scoped_actor self{system};
  auto actor_ref = self->spawn<AddUntilReach3>();

  // send a message to our actor to increment
  self->send(actor_ref, add_atom::value, 1);
  self->send(actor_ref, add_atom::value, 1);
  self->send(actor_ref, add_atom::value, 1);

  std::cout << "Press any char to terminate .." << std::endl;
  std::cin.get();
}

void test_fun_actor(caf::actor_system &system) {
  caf::scoped_actor self{system};

  auto fun_actor_ref = self->spawn(add_until_reach3);
  self->send(fun_actor_ref, add_atom::value, 1);
  self->send(fun_actor_ref, add_atom::value, 1);
  self->send(fun_actor_ref, add_atom::value, 1);

  std::cout << "Press any char to terminate .." << std::endl;
  std::cin.get();
}

void caf_main(caf::actor_system &system) {
  test_cls_actor(system);
  // test_fun_actor(system);
}

CAF_MAIN()
