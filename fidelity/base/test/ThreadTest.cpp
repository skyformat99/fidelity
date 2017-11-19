#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <contract/contract_assert.hpp>

#include <pthread.h>
#include <sched.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/resource.h>

#include <atomic>
#include <chrono>
#include <climits>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <thread>

#include "Definitions.hpp"
#include "SystemAdapterMock.hpp"

#include "../Thread.hpp"

using namespace std::chrono_literals;

namespace t = testing;

namespace fdl {
struct SystemAdapter;
}

namespace fdl::test::thread {

class BASE_ThreadTest : public t::Test {
 public:
  virtual void TearDown() {
    injectSystemAdapter(nullptr);
  }

  static void injectSystemAdapter(std::shared_ptr<SystemAdapter> system) {
    Thread::m_system_di = system;
  }

  size_t m_max_stack = 4096 * 1024 + PTHREAD_STACK_MIN;

  std::unique_ptr<Thread> createThread(const std::string& name, Thread::Type type, int prio,
                                       int affinity, std::function<void()> update,
                                       SystemAdapterMock& system) {
    rlimit limit = {m_max_stack, m_max_stack};
    size_t default_stack_size = 2048 * 1024 + PTHREAD_STACK_MIN;  // default stack size

    EXPECT_CALL(system.threadMock(), hardware_concurrency()).WillOnce(t::Return(2));
    EXPECT_CALL(system.pthreadMock(), pthread_attr_init(t::_));
    EXPECT_CALL(system.resourceMock(), getrlimit(t::_, t::_))
        .WillOnce(t::DoAll(t::SetArgPointee<1>(limit), t::Return(0)));
    EXPECT_CALL(system.pthreadMock(), pthread_attr_setstacksize(t::_, default_stack_size));

    return std::make_unique<Thread>(name, type, prio, affinity, update);
  }

  void checkPeriod(Thread* thread, std::chrono::microseconds period) {
    EXPECT_EQ(period, thread->m_period);
  }
};

DESCRIBE_F(BASE_ThreadTest, constructor, should_check_preconditions) {
  // expect non empty name
  EXPECT_THROW(Thread("", Thread::Type::RT, 1, 0, [] {}),
               std::experimental::contract_violation_error);

  // rt thread expect prio > 0
  EXPECT_THROW(Thread("rt_thread", Thread::Type::RT, 0, 0, [] {}),
               std::experimental::contract_violation_error);

  // rt thread expect prio < 99
  EXPECT_THROW(Thread("rt_thread", Thread::Type::RT, 99, 0, [] {}),
               std::experimental::contract_violation_error);
}

DESCRIBE_F(BASE_ThreadTest, setPeriod, should_set_thread_period) {

  auto system = std::make_shared<SystemAdapterMock>();
  injectSystemAdapter(system);

  auto thread = createThread("thread", Thread::Type::RT, 1, -1, [] {}, *system);

  // expect period > 0
  EXPECT_THROW(thread->setPeriod(0ms), std::experimental::contract_violation_error);

  auto period = 100us;
  thread->setPeriod(period);
  checkPeriod(thread.get(), period);
}

DESCRIBE_F(BASE_ThreadTest, setStackSize, should_set_stack_size) {
  auto system = std::make_shared<SystemAdapterMock>();
  injectSystemAdapter(system);

  auto thread = createThread("thread", Thread::Type::RT, 1, -1, [] {}, *system);

  // ensure size less than max stack limit
  size_t stack = m_max_stack + 1024;
  EXPECT_THROW(thread->setStackSize(stack), std::experimental::contract_violation_error);

  stack = 1048 * 1024;
  EXPECT_CALL(system->pthreadMock(), pthread_attr_setstacksize(t::_, stack + PTHREAD_STACK_MIN));
  thread->setStackSize(stack);
}

DESCRIBE_F(BASE_ThreadTest, create, should_set_scheduler_and_affinity_for_rt_thread) {
  auto system = std::make_shared<SystemAdapterMock>();
  injectSystemAdapter(system);

  const char* name = "rt_thread";
  const int prio = 50;
  const int affinity = 1;
  auto thread = createThread(name, Thread::Type::RT, prio, affinity, [] {}, *system);

  EXPECT_CALL(system->pthreadMock(), pthread_attr_setschedpolicy(t::_, SCHED_FIFO));
  EXPECT_CALL(system->pthreadMock(),
              pthread_attr_setschedparam(t::_, t::Field(&sched_param::sched_priority, prio)));

  auto checkCPU = [](const cpu_set_t* cpu_set) -> int {
    cpu_set_t wanted_cpu_set;
    CPU_ZERO(&wanted_cpu_set);
    CPU_SET(affinity, &wanted_cpu_set);
    return CPU_EQUAL(cpu_set, &wanted_cpu_set);
  };
  EXPECT_CALL(system->pthreadMock(), pthread_attr_setaffinity_np(t::_, t::_, t::Truly(checkCPU)));

  EXPECT_CALL(system->pthreadMock(), pthread_attr_setinheritsched(t::_, PTHREAD_EXPLICIT_SCHED));
  EXPECT_CALL(system->mmanMock(), mlockall(MCL_CURRENT)).Times(2);
  EXPECT_CALL(system->pthreadMock(), pthread_create(t::_, t::_, t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_setname_np(t::_, t::StrEq(name)));
  EXPECT_CALL(system->pthreadMock(), pthread_attr_destroy(t::_));

  thread->create();

  // should be fine
  thread->create();
}

DESCRIBE_F(BASE_ThreadTest, create, should_set_scheduler_and_affinity_for_non_rt_thread) {
  auto system = std::make_shared<SystemAdapterMock>();
  injectSystemAdapter(system);

  const char* name = "non_rt_thread";
  const int affinity = 0;
  auto thread = createThread(name, Thread::Type::NON_RT, 0, affinity, [] {}, *system);

  EXPECT_CALL(system->pthreadMock(), pthread_attr_setschedpolicy(t::_, SCHED_OTHER));
  // not calling pthread_attr_setschedparam for nrt thread

  auto checkCPU = [](const cpu_set_t* cpu_set) -> int {
    cpu_set_t wanted_cpu_set;
    CPU_ZERO(&wanted_cpu_set);
    CPU_SET(affinity, &wanted_cpu_set);
    return CPU_EQUAL(cpu_set, &wanted_cpu_set);
  };
  EXPECT_CALL(system->pthreadMock(), pthread_attr_setaffinity_np(t::_, t::_, t::Truly(checkCPU)));

  EXPECT_CALL(system->pthreadMock(), pthread_attr_setinheritsched(t::_, PTHREAD_EXPLICIT_SCHED));
  // not calling mlockall for nrt thread
  EXPECT_CALL(system->pthreadMock(), pthread_create(t::_, t::_, t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_setname_np(t::_, t::StrEq(name)));
  EXPECT_CALL(system->pthreadMock(), pthread_attr_destroy(t::_));

  thread->create();
}

DESCRIBE_F(BASE_ThreadTest, cancel, should_cancel_the_thread) {
  auto system = std::make_shared<SystemAdapterMock>();
  injectSystemAdapter(system);

  auto thread = createThread("non_rt_thread", Thread::Type::NON_RT, 0, -1, [] {}, *system);

  // not created, should do nothing
  thread->cancel();

  pthread_t pid{1};
  EXPECT_CALL(system->pthreadMock(), pthread_attr_setschedpolicy(t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_attr_setinheritsched(t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_create(t::_, t::_, t::_, t::_))
      .WillOnce(t::DoAll(t::SetArgPointee<0>(pid), t::Return(0)));
  EXPECT_CALL(system->pthreadMock(), pthread_setname_np(t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_attr_destroy(t::_));

  thread->create();

  EXPECT_CALL(system->pthreadMock(), pthread_cancel(pid));
  thread->cancel();
}

DESCRIBE_F(BASE_ThreadTest, join, should_join_the_thread) {
  auto system = std::make_shared<SystemAdapterMock>();
  injectSystemAdapter(system);

  auto thread = createThread("non_rt_thread", Thread::Type::NON_RT, 0, -1, [] {}, *system);

  // not created, should do nothing
  thread->join();

  pthread_t pid{1};
  EXPECT_CALL(system->pthreadMock(), pthread_attr_setschedpolicy(t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_attr_setinheritsched(t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_create(t::_, t::_, t::_, t::_))
      .WillOnce(t::DoAll(t::SetArgPointee<0>(pid), t::Return(0)));
  EXPECT_CALL(system->pthreadMock(), pthread_setname_np(t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_attr_destroy(t::_));

  thread->create();
  thread->stop();

  EXPECT_CALL(system->pthreadMock(), pthread_join(pid, nullptr));
  thread->join();
}

DESCRIBE_F(BASE_ThreadTest, run, should_call_update_on_wake) {
  auto system = std::make_shared<SystemAdapterMock>();
  injectSystemAdapter(system);

  std::atomic<bool> updated{false};
  auto thread = createThread("non_rt_thread", Thread::Type::NON_RT, 0, -1,
                             [&updated] { updated = true; }, *system);

  EXPECT_CALL(system->pthreadMock(), pthread_attr_setschedpolicy(t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_attr_setinheritsched(t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_create(t::_, t::_, t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_setname_np(t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_attr_destroy(t::_));

  thread->create();
  thread->wake();

  void* thread_ptr = thread.get();
  std::future<void> result(std::async([thread_ptr] { Thread::threadRun(thread_ptr); }));
  std::this_thread::sleep_for(5ms);

  thread->stop();
  EXPECT_TRUE(updated);
}

DESCRIBE_F(BASE_ThreadTest, run, should_call_update_if_thread_is_periodic_and_woken_up) {
  auto system = std::make_shared<SystemAdapterMock>();
  injectSystemAdapter(system);

  std::atomic<bool> updated{false};
  auto thread = createThread("non_rt_thread", Thread::Type::NON_RT, 0, -1,
                             [&updated] { updated = true; }, *system);

  EXPECT_CALL(system->pthreadMock(), pthread_attr_setschedpolicy(t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_attr_setinheritsched(t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_create(t::_, t::_, t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_setname_np(t::_, t::_));
  EXPECT_CALL(system->pthreadMock(), pthread_attr_destroy(t::_));

  thread->setPeriod(1ms);
  thread->create();
  thread->wake();
  void* thread_ptr = thread.get();
  std::future<void> result(std::async([thread_ptr] { Thread::threadRun(thread_ptr); }));
  std::this_thread::sleep_for(5ms);

  thread->stop();
  EXPECT_TRUE(updated);
}

}  // namespace fdl::test::thread
