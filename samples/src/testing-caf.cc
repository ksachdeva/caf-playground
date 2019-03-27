#define CAF_SUITE testing_example
#include <caf/all.hpp>
#include <caf/test/dsl.hpp>
#include <caf/test/unit_test_impl.hpp>

caf::behavior actor_under_test(caf::event_based_actor *self) {
  return {[=](int x, int y) { return x + y; }};
}

struct fixture : test_coordinator_fixture<> {};

CAF_TEST_FIXTURE_SCOPE(tracing_scope, fixture)

CAF_TEST(addition) {
  auto an_actor = self->spawn(actor_under_test);
  self->send(an_actor, 2, 3);
  expect((int, int), from(self).to(an_actor).with(2, 3));
  expect((int), from(an_actor).to(self).with(5));
}

CAF_TEST_FIXTURE_SCOPE_END()
