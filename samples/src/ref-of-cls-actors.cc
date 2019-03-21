// Purpose of this sample application is to
// demonstrate how to get the actor handle from with in
// the actor object

// we will again use two actors
//
// One of them would have a trait that would receive the reference to the actor
// i.e. the reference would not be passed at the creation time

#include <iostream>
#include <string>

#include "caf/all.hpp"

using add_atom = caf::atom_constant<caf::atom("add")>;
using sub_atom = caf::atom_constant<caf::atom("sub")>;

using reg_atom = caf::atom_constant<caf::atom("reg")>;

using sub_actor =
    caf::typed_actor<caf::replies_to<sub_atom, int, int>::with<int>>;

using add_actor = caf::typed_actor<

    caf::replies_to<add_atom, int, int>::with<int>,

    // here we define a trait that would accept the handle
    // of sub_actor as a parameter
    caf::reacts_to<reg_atom, sub_actor>

    >;

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
            [=](add_atom, int x, int y) {
              // TODO:
              // How to make sure that sub_actor_ref is valid ?

              std::cout << "We come here in add_atom " << std::endl;

              auto rp = this->make_response_promise<int>();

              // we are going to cheat now by doing sub instead of add
              this->request(this->_sub_actor_ref, caf::infinite,
                            sub_atom::value, x, y)
                  .await([=](int result) mutable {
                    caf::aout(this) << "Acutal result [thanks to sub actor] "
                                    << result << std::endl;

                    // time to deliver on our promise
                    rp.deliver(result);
                  });

              return rp;
            },

            [=](reg_atom, sub_actor &sub_actor_ref) {
              caf::aout(this) << "We did registered the ref" << std::endl;
              this->_sub_actor_ref = caf::actor_cast<sub_actor>(sub_actor_ref);
            }

    };
  }

private:
  sub_actor _sub_actor_ref;
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
            [=](sub_atom, int x, int y) {
              // This one is a bit strange actor,
              // instead of doing its job of simply subtraction
              // it registers itself with add actor as well
              // [Note - this is all for demonstration by creating weird
              // scenarios]

              // send reference to yourself (using 'this')
              // to add actor who (we know) is storing the reference in
              // its private variable.
              //
              // Beacause of this now there is a cyclic reference that has been
              // created and your program will not terminate any more cleanly

              auto rp = this->make_response_promise<int>();
              this->request(this->_add_actor, caf::infinite, reg_atom::value,
                            this)
                  .await([=]() mutable { rp.deliver(x - y); });

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
            caf::aout(self)
                << "Main after sub received " << result << std::endl;

            // let's invoke add here and see if it really adds or use sub
            // (whose reference it has now)

            self->request(add_actor_ref, std::chrono::seconds(10),
                          add_atom::value, 7, 2)
                .receive(
                    [&](int resultx) {
                      caf::aout(self) << "Main after adding received "
                                      << resultx << std::endl;
                    },
                    [&](const caf::error &err) {
                      caf::aout(self)
                          << " [nested adding] errored " << err << std::endl;
                    });
          },
          [&](const caf::error &err) {
            caf::aout(self) << "remote errored " << err << std::endl;
          });

  std::cout << "Press any char to terminate .." << std::endl;
  std::cin.get();
}

CAF_MAIN()
