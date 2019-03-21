// This example is similar yet different from self-quiting-actor
//
// here we would try to spawn the actor and keep its reference in some
// collection
//
// The actor that would keep the cache would be responsible for removing from
// cache

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
            this->quit(caf::exit_reason::user_shutdown);
          }
        }};
  }

private:
  // state is maintained in the class based actor
  int _value;
};

class TheActorCacher {
public:
  TheActorCacher(uptill_3_adder &adder) {
    this->_adders.push_back(std::move(adder));
  }

  void eraseIt() { this->_adders.clear(); }

private:
  std::vector<uptill_3_adder> _adders;
};

void caf_main(caf::actor_system &system) {
  caf::scoped_actor self{system};
  auto actor_ref = self->spawn<AddUntilReach3>();

  actor_ref->attach_functor([&](const caf::error &reason) {
    std::cout << " exited: " << caf::to_string(reason) << std::endl;
  });

  // send a message to our actor to increment
  self->send(actor_ref, add_atom::value, 1);
  self->send(actor_ref, add_atom::value, 1);
  self->send(actor_ref, add_atom::value, 1);

  std::cout << "Press any char to terminate .." << std::endl;
  std::cin.get();
}

CAF_MAIN()
