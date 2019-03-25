#include <caf/all.hpp>
#include <iostream>
#include <string>

// This is an event based actor that does something that takes
// long time before it returns its result
caf::behavior takes_long_time_to_respond(caf::event_based_actor *self) {
  return {

      // behaviors of our actor

      // only behavior this implement is a function
      // that takes nothing but returns a value
      // however, it does computation heavy stuff before returning its
      // result
      [=](int x) -> int {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return 5 * x;
      }};
}

// This is an implementation of your typical actor that would send a message
// to a buddy actor. Now the buddy actor is kind of slow in responding back.
//
// Also this actor implements more than 1 behavior.
//
// What we are trying to learn/demonstrate here is that while SomeOther actor
// is waiting for a response from buddy can we invoke some other behavior
caf::behavior some_other_actor(caf::event_based_actor *self,
                               const caf::actor &buddy) {

  return {

      // behaviors of our actor

      // this is a behavior 1 that simply receives x and multiply it with 2
      [=](int x) -> int {
        caf::aout(self) << "In Behavior 1 " << std::endl;
        return 2 * x;
      },

      // this is a behavior 2 that simply receives x and returns it with some
      // prefix
      [=](const std::string &x) -> std::string {
        caf::aout(self) << "In Behavior 2 " << std::endl;
        return "Hey " + x;
      },

      // this is a behavior 3 that sends message to buddy actor and waits for
      // its response
      //
      // While waiting for the response, this behavior decides to use 'await'
      // instead of 'then'. 'Await' indiciates to the framework (CAF) that it
      // should send messages to other behaviors of this actor while it is
      // waiting to receive the response from buddy !
      [=](caf::get_atom) {
        self->request(buddy, caf::infinite, 10).await([=](int result) {
          caf::aout(self) << "Received result from buddy " << result
                          << std::endl;
        });
      }};
}

void caf_main(caf::actor_system &system) {

  caf::scoped_actor self{system};

  auto buddy = self->spawn(takes_long_time_to_respond);
  auto some_one = self->spawn(some_other_actor, buddy);

  // time to send messages
  //
  // Again what we intend to learn here is that we would issue
  // message to behavior 3 of some_one and we know that it would long
  // time. In the mean time we would send message to its other behaviors
  // and see if they respond back before we get result from behavior 3.
  //
  // Since we are using 'await' we expect that even though we sent the messages
  // to behavior 1 and 2, they would get triggered/processed only after
  // behavior 3 is processed

  self->send(some_one, caf::get_atom::value);

  // above call is in progress, we would now invoke other behaviors
  // and check if they would be invoked or not while we wait for behavior 1 to
  // finish
  self->send(some_one, "SomeMessage"); // behavior 2

  self->send(some_one, 4); // behavior 1

  // If you see "In behavior 2" and "In Behavior 1" then it would
  // mean that these messages were sent and processed while we were waiting
  // for long running behavior 3. Now this would not be the expected behavior !
  //
  // Run it to verify and make sure to wait for few seconds before pressing the
  // enter
  //

  std::cout << "Press any char to terminate .." << std::endl;
  std::cin.get();
}

CAF_MAIN()
