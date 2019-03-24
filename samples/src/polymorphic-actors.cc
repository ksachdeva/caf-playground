// This example shows how one could have different implementation
// of a trait and therefore achieve the polymorphism.

#include <iostream>
#include <string>

#include <caf/all.hpp>

using get_edges_atom = caf::atom_constant<caf::atom("get_edges")>;

// This is our interface definition that
// various actors should implement
using shape_actor =
    caf::typed_actor<caf::replies_to<get_edges_atom>::with<int>>;

// implementing shape_actor using the functions (statically typed and event
// based)
//
// We have 3 types of actors that implement the same interface

shape_actor::behavior_type circle_actor_fn() {
  return {[](get_edges_atom) { return 0; }};
};

shape_actor::behavior_type triangle_actor_fn() {
  return {[](get_edges_atom) { return 3; }};
};

shape_actor::behavior_type square_actor_fn() {
  return {[](get_edges_atom) { return 4; }};
};

// Now we build an actor that will be supplied one
// of them and it would query the number of edges
// using that

void query_shapes(caf::event_based_actor *self, const shape_actor &shape) {
  self->request(shape, std::chrono::seconds(10), get_edges_atom::value)
      .then([=](const int &edges_count) {
        caf::aout(self) << edges_count << std::endl;
      });
}

void caf_main(caf::actor_system &system) {
  // create all of our shape actors
  auto circle_actor = system.spawn(circle_actor_fn);
  auto triangle_actor = system.spawn(triangle_actor_fn);
  auto square_actor = system.spawn(square_actor_fn);

  system.spawn(query_shapes, circle_actor);
  system.spawn(query_shapes, triangle_actor);
  system.spawn(query_shapes, square_actor);
}

CAF_MAIN()