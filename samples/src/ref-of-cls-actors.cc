// Purpose of this sample application is to
// demonstrate how to get the actor handle from with in
// the actor object

// We will use two actors here to demonstrate the concept
//
// One of them (SubActor) would have a method that would receive the reference
// to the other actor (AddActor) at the construction time and would store it as
// a reference in the field of the class.

#include <iostream>
#include <string>

#include <caf/all.hpp>

using add_atom = caf::atom_constant<caf::atom("add")>;
using sub_atom = caf::atom_constant<caf::atom("sub")>;
using reg_atom = caf::atom_constant<caf::atom("reg")>;

using sub_actor = caf::typed_actor<

    // this message performs simple subtraction
    caf::replies_to<sub_atom, int, int>::with<int>

    >;

using add_actor = caf::typed_actor<

    caf::replies_to<add_atom, int, int>::with<int>,

    // here we define a trait that would accept the reference/handle
    // of sub_actor as a parameter
    caf::reacts_to<reg_atom, sub_actor>

    >;

class ActorThatCheatsWhenAdding : public add_actor::base {
public:
  ActorThatCheatsWhenAdding(caf::actor_config &cfg) : add_actor::base(cfg) {}
  ~ActorThatCheatsWhenAdding() {
    std::cout << "ActorThatCheatsWhenAdding Destructor called" << std::endl;
  }

public:
  const char *name() const override { return "ActorThatCheatsWhenAdding"; }

  ActorThatCheatsWhenAdding::behavior_type make_behavior() override {
    return {

        // implementation of behaviors

        // behavior responsible for adding the provided inputs
        [=](add_atom, int x, int y) {
          auto rp = this->make_response_promise<int>();

          // we are going to cheat now by doing sub instead of add
          this->request(this->_sub_actor_ref, caf::infinite, sub_atom::value, x,
                        y)
              .await([=](int result) mutable {
                caf::aout(this) << "Acutal result [thanks to sub actor] "
                                << result << std::endl;

                // time to deliver on our promise
                rp.deliver(result);

                // after doing its sneaky work, this actor
                // would now quit
                //
                // Normally you would not do this in real code,
                // you may send a separate message to make this actor quit
                // but for our sample we are calling it here
                //
                // Why calling it is essential for this convoluted sample ?
                //
                // Since we have created a cyclic reference (AddActor has ref to
                // SubActor and vice versa) the process will not cleanly exit
                // anymore.
                //
                // By calling quit here we would trigger the on_exit method of
                // this actor which then destroys the reference to sub actor
                // that it holds and essentially ending in breaking the cycle !

                this->quit();
              });

          return rp;
        },

        // behavior responsible for registering the provided reference
        [=](reg_atom, sub_actor &sub_actor_ref) {
          caf::aout(this) << "We did registered the ref" << std::endl;
          this->_sub_actor_ref = sub_actor_ref;
        }

    };
  }

  void on_exit() override {
    caf::aout(this) << "on_exit[ActorThatCheatsWhenAdding]" << std::endl;
    destroy(this->_sub_actor_ref);
  }

private:
  sub_actor _sub_actor_ref;
};

// A class based sub_actor that knows about add_actor at the construction
// time and store its reference
class ActorThanCanSub : public sub_actor::base {
public:
  ActorThanCanSub(caf::actor_config &cfg, add_actor &add_actor_ref)
      : sub_actor::base(cfg), _add_actor(add_actor_ref),
        _have_registered_itself_with_adder(false) {}
  ~ActorThanCanSub() {
    std::cout << "ActorThanCanSub Destructor called" << std::endl;
  }

public:
  const char *name() const override { return "ActorThanCanSub"; }

  ActorThanCanSub::behavior_type make_behavior() override {
    return {

        // implementation of its behavior

        // implementation of subtraction behavior
        [=](sub_atom, int x, int y) {
          // This one is a bit strange actor,
          // instead of doing its job of simply subtraction
          // it registers itself with add actor
          // [Note - this is all for demonstration by creating weird
          // scenarios]

          if (!_have_registered_itself_with_adder) {

            // sends reference to itself (using 'this')
            // to add actor who (we know) is storing the reference in
            // its private variable.
            //
            // Beacause of this now there is a cyclic reference that has been
            // created and your program will not terminate any more cleanly
            //
            // How to clean up this cross reference will be shown later or in
            // other example

            // since the call for registration will be asynchronous
            // we would construct a promise
            auto rp = this->make_response_promise<int>();

            this->request(this->_add_actor, caf::infinite, reg_atom::value,
                          this)
                .await([=]() mutable {
                  // reaching here means that reference to sub_actor was
                  // successfully provided to the add actor
                  this->_have_registered_itself_with_adder = true;

                  // deliver the actual subtraction result
                  rp.deliver(x - y);
                });

            return rp;
          }
          // perform its task of doing plain old subtraction
          //
          // Now since we return a promise this time around we need to use
          // a helper to make a full filled promise
          return this->response(x - y);
        }};
  }

  void on_exit() override {
    caf::aout(this) << "on_exit[ActorThanCanSub]" << std::endl;
    destroy(this->_add_actor);
  }

private:
  add_actor _add_actor;
  bool _have_registered_itself_with_adder;
};

void caf_main(caf::actor_system &system) {
  caf::scoped_actor self{system};

  // we create an actor that can add
  auto add_actor_ref = self->spawn<ActorThatCheatsWhenAdding>();

  // we create an actor that can do sub
  // and will want to register itself with add actor at some point
  // in future so we provide it the reference to our add actor
  auto sub_actor_ref = self->spawn<ActorThanCanSub>(add_actor_ref);

  self->request(sub_actor_ref, std::chrono::seconds(10), sub_atom::value, 4, 1)
      .receive(

          // we attach handlers (lambda) to receive the result/promise from
          // our message sent to sub_actor and we wait for 10 seconds to let
          // it do its business

          [&](int result) {
            caf::aout(self)
                << "Main after sub received " << result << std::endl;

            // Note we have a nested callback (pyramid of hell here)
            //
            // It is not a good idea to write code like this but for
            // demonstration purposes it is done this way for now.

            // let's invoke add here and see if it really adds or uses sub
            // (whose reference it has now)

            self->request(add_actor_ref, std::chrono::seconds(10),
                          add_atom::value, 7, 2)
                .receive(

                    // we attach handlers (lambda) to receive the result/promise
                    // from our message sent to add_actor and we wait for 10
                    // seconds to let it do its business.
                    //
                    // If it can not do its buisness with in 10 seconds then the
                    // error handler will be invoked

                    [&](int resultx) {
                      caf::aout(self) << "Main after adding received "
                                      << resultx << std::endl;
                    },

                    // This is the error handler for the second (nested) receive
                    // in this pyramid of callback hell
                    [&](const caf::error &err) {
                      caf::aout(self)
                          << " [nested adding] errored " << err << std::endl;
                    });
          },

          // This is the error handler for the first receive in this pyramid of
          // callback hell
          [&](const caf::error &err) {
            caf::aout(self) << "remote errored " << err << std::endl;
          });

  std::cout << "Press any char to terminate .." << std::endl;
  std::cin.get();
}

CAF_MAIN()
