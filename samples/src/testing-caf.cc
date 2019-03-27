#define CAF_SUITE testing_example
#include <caf/all.hpp>
#include <caf/test/dsl.hpp>
#include <caf/test/unit_test_impl.hpp>

#define ERROR_HANDLER [&](caf::error &err) { CAF_FAIL(sys.render(err)); }

struct config : caf::actor_system_config {
  config() { set("scheduler.policy", caf::atom("testing")); }
};

caf::behavior actor_under_test(caf::event_based_actor *self) {
  return {[=](int x, int y) {
    caf::aout(self) << "entered in behavior" << std::endl;
    return x + y;
  }};
}

CAF_TEST_FIXTURE_SCOPE(tracing_scope, test_coordinator_fixture<config>)

CAF_TEST(divide) {
  auto an_actor = self->spawn(actor_under_test);

  sched.run();
  sched.inline_next_enqueue();
  self->request(an_actor, std::chrono::seconds(10), 2, 3)
      .receive(
          [&](int result) {
            caf::aout(self) << "Result is " << result << std::endl;
          },
          ERROR_HANDLER);

  caf::aout(self) << "prints only after actor has processed request"
                  << std::endl;
}

CAF_TEST_FIXTURE_SCOPE_END()
