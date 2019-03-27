// This example tries to build scenario described below -:
//
// You have worker actors. As the name implies they do some work.
// All of them are equally capable and able to do the same type of work.
//
//
// You have a manager actor. As the name implies it would do some management.
// In this example, the management is about
//
//  - maintaining the list of workers
//  - assigning a worker to the customer
//  - once customers work is done, unassigning that work and make it free
//  - passing the work from customer to the worker assigned for that customer
//
//
// We will customer actors as well. Now these customer actors will not be
// directly aware of which worker was assigned to do their work. They will
// always go via the manager actor.

#include <caf/all.hpp>

using work_rr_atom = caf::atom_constant<caf::atom("work_rr")>;

// let's define a trait for our worker actors
using worker_actor = caf::typed_actor<

    // only trait required for our worker
    caf::replies_to<work_rr_atom, std::string>::with<std::string>

    >;

// Traits for our manager actor
using manager_reg_worker_atom = caf::atom_constant<caf::atom("man_reg")>;
using manager_assign_customer_atom = caf::atom_constant<caf::atom("man_a_cus")>;
using manager_unassign_customer_atom =
    caf::atom_constant<caf::atom("man_u_cus")>;
using manager_pass_work_atom = caf::atom_constant<caf::atom("man_p_work")>;

using manager_actor = caf::typed_actor<

    // register a worker actor
    caf::reacts_to<manager_reg_worker_atom, worker_actor>,

    // assign a customer to the first available worker actor
    // a customer is identified using customer_id
    caf::reacts_to<manager_assign_customer_atom, std::string>,

    // unassign the allocated worker for the customer
    caf::reacts_to<manager_unassign_customer_atom, std::string>,

    // ask the worker to do some work
    caf::replies_to<manager_pass_work_atom, std::string,
                    std::string>::with<std::string>

    >;

// Finally the trait for our customer actors
using customer_rr_atom = caf::atom_constant<caf::atom("cust_rr")>;

using customer_actor = caf::typed_actor<

    // only trait required for our worker
    caf::replies_to<customer_rr_atom, std::string>::with<std::string>

    >;

// Implementation of various actors
worker_actor::behavior_type make_worker_actor(worker_actor::pointer self,
                                              int worker_id) {

  return {

      // the behaviors of this actor

      // the only behavior. We keep it simply by simply reversing the passed
      // strings
      [=](work_rr_atom, const std::string &some_work) {
        caf::aout(self) << "Worker " << worker_id << " is working !"
                        << std::endl;
        return std::string(some_work.rbegin(), some_work.rend());
      }

  };
}

struct WorkerInfo {
  bool is_assigned;
  worker_actor wa;
};

struct ManagerState {
  // this keeps the state
  std::vector<WorkerInfo> workers;

  // cache to access the worker actor quickly
  std::map<std::string, worker_actor> actors_cache;
};

class ManagerActor : public manager_actor::base {
public:
  ManagerActor(caf::actor_config &cfg) : manager_actor::base(cfg) {}
  ~ManagerActor() { std::cout << "Destroying ManagerActor" << std::endl; }

private:
  ManagerState state;

public:
  ManagerActor::behavior_type make_behavior() {

    return {

        [=](manager_reg_worker_atom, const worker_actor &wa) {
          caf::aout(this) << "Registering worker " << std::endl;
          this->state.workers.emplace_back(WorkerInfo{false, wa});
        },

        [=](manager_assign_customer_atom, const std::string &customer_id) {
          for (auto &wi : this->state.workers) {
            if (!wi.is_assigned) {
              caf::aout(this)
                  << "Assigning " << customer_id << " a worker" << std::endl;
              wi.is_assigned = true;
              this->state.actors_cache[customer_id] = wi.wa;
              break;
            }
          }
        },

        [=](manager_unassign_customer_atom, const std::string &customer_id) {
          auto wa = this->state.actors_cache[customer_id];

          for (auto &wi : this->state.workers) {
            if (wa == wi.wa) {
              caf::aout(this)
                  << "unassigning worker for " << customer_id << std::endl;
              wi.is_assigned = false;
              this->state.actors_cache.erase(customer_id);
            }
          }
        },

        [=](manager_pass_work_atom, const std::string &customer_id,
            const std::string &message) {
          if (this->state.actors_cache.count(customer_id) == 0) {
            caf::aout(this)
                << "No worker assigned for this customer " << std::endl;
            return this->response(std::string("sorry no one available !"));
          }

          auto wa = this->state.actors_cache[customer_id];

          auto rp = this->make_response_promise<std::string>();
          this->request(wa, caf::infinite, work_rr_atom::value, message)
              .then([=](const std::string &resp) mutable { rp.deliver(resp); });

          return rp;
        }

    };
  }
};

class CustomerActor : public customer_actor::base {
public:
  CustomerActor(caf::actor_config &cfg, const std::string &customer_id,
                const manager_actor &ma)
      : customer_actor::base(cfg), _customer_id(customer_id), _ma(ma) {}
  ~CustomerActor() { std::cout << "Destroying CustomerActor" << std::endl; }

private:
  manager_actor _ma;
  std::string _customer_id;

public:
  CustomerActor::behavior_type make_behavior() {

    return {

        // the behaviors of this actor

        // the only behavior.
        [=](customer_rr_atom, const std::string &some_work) {
          // Customer is going to send the work to the manager who
          // will then send it to the worker assigned to this customer

          auto rp = this->make_response_promise<std::string>();

          this->request(_ma, caf::infinite, manager_pass_work_atom::value,
                        this->_customer_id, some_work)
              .then([=](const std::string &response) mutable {
                rp.deliver(response);
              });

          return rp;
        }};
  }
};

void caf_main(caf::actor_system &system) {
  caf::scoped_actor self{system};

  // let's spawn 4 worker actors
  auto worker_actor_1 = self->spawn(make_worker_actor, 1);
  auto worker_actor_2 = self->spawn(make_worker_actor, 2);
  auto worker_actor_3 = self->spawn(make_worker_actor, 3);
  auto worker_actor_4 = self->spawn(make_worker_actor, 4);

  // spawn 1 manager actor
  auto manager_actor_1 = self->spawn<ManagerActor>();

  std::string customer_1_id("customer_1");
  std::string customer_2_id("customer_2");

  // spawn 2 Customer Actors
  auto customer_actor_1 =
      self->spawn<CustomerActor>(customer_1_id, manager_actor_1);
  auto customer_actor_2 =
      self->spawn<CustomerActor>(customer_2_id, manager_actor_1);

  // now that relationship between customer and manager is established
  // we will register/assign our workers with manager

  self->request(manager_actor_1, caf::infinite, manager_reg_worker_atom::value,
                worker_actor_1);
  self->request(manager_actor_1, caf::infinite, manager_reg_worker_atom::value,
                worker_actor_2);
  self->request(manager_actor_1, caf::infinite, manager_reg_worker_atom::value,
                worker_actor_3);
  self->request(manager_actor_1, caf::infinite, manager_reg_worker_atom::value,
                worker_actor_4);

  // let's assign a worker to a customer
  self->request(manager_actor_1, caf::infinite,
                manager_assign_customer_atom::value, customer_1_id);

  // now we send a message to a customer which delegates to manager
  // to manager which delegates to worker
  self->request(customer_actor_1, caf::infinite, customer_rr_atom::value,
                "work_from_cust_1")
      .receive(
          [=](const std::string &work_response) {
            // caf::aout(self) << work_response << std::endl;
            std::cout << "Response for first request - " << work_response
                      << std::endl;
          },
          [=](const caf::error &err) {
            std::cout << "Errored 1" << std::endl;
          });

  // unassign the worker for customer 1
  self->request(manager_actor_1, caf::infinite,
                manager_unassign_customer_atom::value, customer_1_id);

  // now try to send a message to a customer which should fail in some
  // way as there is no worker assigned
  self->request(customer_actor_1, caf::infinite, customer_rr_atom::value,
                "additional_work_from_cust_1")
      .receive(
          [=](const std::string &work_response) {
            // caf::aout(self) << work_response << std::endl;
            std::cout << "Response for second request - " << work_response
                      << std::endl;
          },
          [=](const caf::error &err) {
            std::cout << "Errored 2" << std::endl;
          });

  std::cout << "Press any char to terminate .." << std::endl;
  std::cin.get();
}

CAF_MAIN()
