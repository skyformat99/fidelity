#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <pthread.h>
#include <sched.h>
#include <stddef.h>

#include <climits>
#include <functional>
#include <string>

#include <fidelity/base/Thread.hpp>
#include <fidelity/base/test/Definitions.hpp>

using namespace std::chrono_literals;

namespace t = testing;

namespace fdl::test::pthread_scenario {

class BASE_PthreadScenario : public t::Test {
 public:
  void checkProperties(Thread& thread, const char* name, Thread::Type type, int prio,
                       int affinity) {
    char p_name[16]{0};
    pthread_getname_np(thread.m_thread, p_name, sizeof(p_name));
    EXPECT_STREQ(name, p_name);

    pthread_attr_t attr{};
    pthread_attr_init(&attr);
    pthread_getattr_np(thread.m_thread, &attr);

    size_t stack_size{0};
    pthread_attr_getstacksize(&attr, &stack_size);
    EXPECT_EQ(stack_size, thread.DEFAULT_STACK_SIZE + PTHREAD_STACK_MIN);

    int policy{0};
    sched_param param{0};
    pthread_attr_getschedpolicy(&attr, &policy);
    pthread_attr_getschedparam(&attr, &param);

    if (type == Thread::Type::RT) {
      EXPECT_EQ(SCHED_FIFO, policy);
    } else {
      EXPECT_EQ(SCHED_OTHER, policy);
    }
    EXPECT_EQ(prio, param.sched_priority);

    pthread_attr_destroy(&attr);

    if (affinity >= 0) {
      cpu_set_t cpu_set{};
      CPU_ZERO(&cpu_set);
      pthread_getaffinity_np(thread.m_thread, sizeof(cpu_set), &cpu_set);

      cpu_set_t wanted_cpu_set;
      CPU_ZERO(&wanted_cpu_set);
      CPU_SET(affinity, &wanted_cpu_set);
      EXPECT_TRUE(CPU_EQUAL(&cpu_set, &wanted_cpu_set));
    }
  }
};

DESCRIBE_F(BASE_PthreadScenario, rt_thread, should_have_valid_properties) {
  std::string name("rt_thread");
  int prio = 97;
  int affinity = 0;

  Thread thread{name, Thread::Type::RT, prio, affinity, [] {}};
  thread.create();

  checkProperties(thread, name.c_str(), Thread::Type::RT, prio, affinity);

  thread.stop();
  thread.join();
}

DESCRIBE_F(BASE_PthreadScenario, non_rt_thread, should_have_valid_properties) {
  std::string name("non_rt_thread");
  int prio = 0;
  int affinity = 0;

  Thread thread{name, Thread::Type::NON_RT, prio, affinity, [] {}};
  thread.create();

  checkProperties(thread, name.c_str(), Thread::Type::NON_RT, prio, affinity);

  thread.stop();
  thread.join();
}

}  // namespace fdl::test::pthread_scenario
