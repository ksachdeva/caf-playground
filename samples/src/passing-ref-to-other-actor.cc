// this sample program shows various aspects of coordinating between
// two actors

// How to pass a reference of an actor to another at construction time
// How to invoke another actor and wait for its response before delivering the
// response from it

#include <iostream>
#include <string>

#include "caf/all.hpp"

using add_atom = caf::atom_constant<caf::atom("add")>;
using sub_atom = caf::atom_constant<caf::atom("sub")>;

using add_actor =
    caf::typed_actor<caf::replies_to<add_atom, int, int>::with<int>>;

using sub_actor =
    caf::typed_actor<caf::replies_to<sub_atom, int, int>::with<int>>;

// A class based add_actor
class ActorThatCanAdd : public add_actor::base {
public:
  ActorThatCanAdd(caf::actor_config &cfg) : add_actor::base(cfg) {}
  ~ActorThatCanAdd() {
    std::cout << "ActorThatCanAdd Destructor called" << std::endl;
  }

public:
  ActorThatCanAdd::behavior_type make_behavior() {
    return {// the behaviors for this actor
            [=](add_atom, int x, int y) { return x + y; }};
  }
};

// A class based sub_actor that knows about add_actor at the construction
// time and store its reference
class ActorThanCanSub : public sub_actor::base {
public:
  ActorThanCanSub(caf::actor_config &cfg, add_actor &add_actor_ref)
      : sub_actor::base(cfg), _add_actor(add_actor_ref) {}
  ~ActorThanCanSub() {
    std::cout << "ActorThanCanSub Destructor called" << std::endl;
  }

public:
  ActorThanCanSub::behavior_type make_behavior() {
    return {// the behaviors for this actor
            [=](sub_atom, int x, int y) -> caf::result<int> {
              // this actor is bit strange
              // before it does the subtraction it makes
              // use of Add actor to add some values to x
              //
              // and only after that would invoke the subtraction operation
              caf::aout(this) << "Passed value of x and y are - " << x << " "
                              << y << " result without add would have been "
                              << (x - y) << std::endl;

              // since we are now invoking the call to another actor
              // which is going to be async (however we will not allow other
              // handlers to be invoked while it is going on)
              //
              // It is async but we need to return something from here. We
              // simply return a 'promise' A promise that I would return you the
              // value in near future. You Mr. Caller please wait for that
              // promise to be resolved

              auto rp = this->make_response_promise<int>();

              this->request(this->_add_actor, caf::infinite, add_atom::value, x,
                            2)
                  .await([=](int result) mutable {
                    int sub_result = result - y;

                    caf::aout(this) << "Acutal sub result [thanks to add actor "
                                       "that added 2 to x ] is "
                                    << sub_result << std::endl;

                    // time to deliver on our promise
                    rp.deliver(sub_result);
                  });

              return rp;
            }};
  }

private:
  add_actor _add_actor;
};

void caf_main(caf::actor_system &system) {
  caf::scoped_actor self{system};

  // we create an actor that can add
  auto add_actor_ref = self->spawn<ActorThatCanAdd>();

  // we create an actor that can do sub but also wants to do add sometimes
  // and hence it requires a reference to add actor
  auto sub_actor_ref = self->spawn<ActorThanCanSub>(add_actor_ref);

  self->request(sub_actor_ref, std::chrono::seconds(10), sub_atom::value, 4, 1)
      .receive(
          [&](int result) {
            caf::aout(self) << "Main received " << result << std::endl;
          },
          [&](const caf::error &err) {
            caf::aout(self) << "remote errored " << err << std::endl;
          });

  std::cout << "Press any char to terminate .." << std::endl;
  std::cin.get();
}

CAF_MAIN()
