// This is a traditional hello world example with a twist
//
// Here we have two Actors

// Reverser - This actor has the super powers of taking any string and returning
// the reverse of it

// SomeOne - An actor that sends message to Reverser and expects
// the reversed version of the message it sent

#include <caf/all.hpp>
#include <iostream>
#include <string>

// The very first thing we want to do is to define the trait
// for Reverser. If you come from C#/Java languages you can think of
// trait (conceptually) as an interface.

using reverser_actor = caf::typed_actor<
    // the trait consist only of one method that takes string as an input
    // and returns string as an output
    caf::replies_to<std::string>::with<std::string>>;

// Implementation of actor can be done using a "function"
// or "class". We will explore both here. Normally for use case
// like the one we have here function based actors are good enough.

// ======= FUNCTION BASED IMLPEMENTATION OF reverser_actor trait

// Return type of function based actors is <trait_name>::behavior_type
// There is one mandatory (first argument) of the function based actors
// and it is the pointer the actor. What happens really behind the scenes
// in the framework is that it creates the 'object' and expects your function
// to provide the lambdas that comply with the trait. The framework then
// attaches these lambdas to the object. In other words, it is the framework
// that will create (spawn) the object corresponidng to your actor.

reverser_actor::behavior_type mighty_reverser(reverser_actor::pointer self) {
  return {

      // Return a set of lambda functions that comply with the
      // definition of your trait. Here we just have one method
      // that accepts string as an input and returns string as
      // an output

      [=](const std::string &what) -> std::string {
        caf::aout(self) << "[FN] " << what << std::endl;
        return std::string(what.rbegin(), what.rend());
      }};
}

// ======= CLASS BASED IMPLEMENTATION OF reverser_actor trait

class TheMightyReverser : public reverser_actor::base {
public:
  TheMightyReverser(caf::actor_config &cfg) : reverser_actor::base(cfg) {}

  // make_behavior is the mandatory method that you must
  // implement

  TheMightyReverser::behavior_type make_behavior() {
    // This block will feel a lot like the one that we created for
    // the function based implementation of our actor
    //
    // Note how we now have access to 'this' and therefore
    // all things you can traditionally access using `this` in
    // an object (methods and fields) you can do now.

    return {

        // Return a set of lambda functions that comply with the
        // definition of your trait. Here we just have one method
        // that accepts string as an input and returns string as
        // an output

        [=](const std::string &what) -> std::string {
          caf::aout(this) << "[CLS] " << what << std::endl;
          return std::string(what.rbegin(), what.rend());
        }};
  }
};

// ========= SomeOne actor
//
// This actor does not implement any trait per say
// and we will simply implement it using function based approach
//
// However, SomeOne requires a reference to actor that implements
// reverser_trait so we would pass that as the second argument
//
// Note - the second argument is actually the trait (think interface)
// instead of the actual implementation. This gives you the freedom
// to pass any implementation (as long as it is compliant with this trait)

void some_one(caf::event_based_actor *self, const reverser_actor &reverser) {

  // finally, the next line here shows how a request from actor to another
  // actor is made. The first argument is the refernce (handle) to the actor
  // to whom the message is to be sent. The second argument is for how long
  // the requester will wait for the reply and the rest are arguments of the
  // the method being invoked. In this case we only have one input string so
  // we (as customary) send 'hello world'.
  //
  // Since the messaging is asynchronous our response would come some time
  // later. In order to receive the return value we here attach a lambda
  // with the help of 'then' method.
  //
  // What is most interesting about CAF is that all we do here is
  // not only type-safe but under the governenance of compiler.
  //
  // Try changing the input or output types in below code and compiler
  // will send messages you way that you would think twice about doing
  // such a thing :)

  self->request(reverser, std::chrono::seconds(10), "Hello World!")
      .then([=](const std::string &what) {
        caf::aout(self) << what << std::endl;
      });
}

void caf_main(caf::actor_system &system) {
  caf::scoped_actor self{system};

  // let's create our mighty reverser using function based
  // approach
  auto fn_mighty_reverser_actor = self->spawn(mighty_reverser);

  // the class based implementation is created using a similar way
  auto cls_mighty_reverser_actor = self->spawn<TheMightyReverser>();

  // now we need to spawn Mr. SomeOne as well
  //
  // it should be clear from the implementation of some_one that
  // as it spawns it would send the request to mighty_reverser
  auto some_one_actor = self->spawn(some_one, fn_mighty_reverser_actor);

  // we spawn some_one again but this time with the refernce to the class
  // based reverser
  auto other_one_actor = self->spawn(some_one, cls_mighty_reverser_actor);

  // here all our actors are going out of scope (RAII) and therefore
  // will go away and free our memory in peace
}

CAF_MAIN()