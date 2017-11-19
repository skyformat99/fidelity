#include "Thread.hpp"

#include <sched.h>
#include <sys/mman.h>
#include <sys/resource.h>

#include <contract/contract_assert.hpp>

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <thread>

#include "PrioMutex.hpp"
#include "SystemAdapter.hpp"

using namespace std::chrono_literals;

constexpr int SCHED_RT = SCHED_FIFO;
constexpr int SCHED_NON_RT = SCHED_OTHER;

// monitor realtime behavior
// assert if new is called in rt thread
void* operator new(size_t size) {  // NOLINT
  pthread_attr_t attr{};
  ENSURE(pthread_attr_init(&attr) == 0);
  ENSURE(pthread_getattr_np(pthread_self(), &attr) == 0);

  int policy{0};
  ENSURE(pthread_attr_getschedpolicy(&attr, &policy) == 0);
  ENSURE(pthread_attr_destroy(&attr) == 0);
  ENSURE(policy == SCHED_NON_RT, "Memory allocation not allowed in realtime thread.");

  return std::malloc(size);  // NOLINT
}

namespace fdl {

std::shared_ptr<SystemAdapter> Thread::m_system_di{nullptr};

Thread::Thread(const std::string& name, Thread::Type type, int prio, int affinity,
               std::function<void()> update)
    : m_name(name), m_type(type), m_prio(prio), m_affinity(affinity), m_update(std::move(update)) {

  EXPECT(!name.empty(), "Thread needs to be named.");

  if (Thread::m_system_di != nullptr) {
    m_system = Thread::m_system_di;
  } else {
    m_system = std::make_shared<SystemAdapter>();
    ENSURE(m_system != nullptr);
  }

  if (m_type == Thread::Type::RT) {
    EXPECT(prio > 0 && prio < 99, "realtime threads priority needs a be between 0 and 99.");
  }

  ENSURE(m_affinity < static_cast<int>(m_system->thread->hardware_concurrency()),
         "Affinity does not match available CPUs.");

  ENSURE(m_system->pthread->pthread_attr_init(&m_pthread_attr) == 0,
         "Could not initialize thread attributes.");

  rlimit limit{};
  ENSURE(m_system->resource->getrlimit(RLIMIT_STACK, &limit) == 0);
  ENSURE(limit.rlim_cur > PTHREAD_STACK_MIN);
  m_max_stack_size = limit.rlim_cur - PTHREAD_STACK_MIN;

  setStackSize(Thread::DEFAULT_STACK_SIZE);
}

void Thread::setPeriod(std::chrono::microseconds period) {
  EXPECT(period > 0ms);
  // only set if thread is not created
  if (!m_created) {
    m_period = period;
  }
}

void Thread::setStackSize(size_t size) {
  // only set if thread is not created
  if (!m_created) {
    ENSURE(size <= m_max_stack_size, "Could not set stack size.");
    ENSURE(m_system->pthread->pthread_attr_setstacksize(&m_pthread_attr,
                                                        size + PTHREAD_STACK_MIN) == 0,
           "Could not set stack size.");
  }
}

void Thread::setSched() {
  if (m_type == Type::RT) {
    struct sched_param param {};
    param.sched_priority = m_prio;
    ENSURE(m_system->pthread->pthread_attr_setschedpolicy(&m_pthread_attr, SCHED_RT) == 0,
           "Could not set rt scheduler.");
    ENSURE(m_system->pthread->pthread_attr_setschedparam(&m_pthread_attr, &param) == 0,
           "Could not set thread priority.");
  } else {
    ENSURE(m_system->pthread->pthread_attr_setschedpolicy(&m_pthread_attr, SCHED_NON_RT) == 0,
           "Could not set non rt scheduler");
  }
}

void Thread::setAffinity() {
  // negative value means no affinity wanted
  if (m_affinity >= 0) {
    cpu_set_t set{};
    CPU_ZERO(&set);
    CPU_SET(m_affinity, &set);  // NOLINT
    ENSURE(m_system->pthread->pthread_attr_setaffinity_np(&m_pthread_attr, sizeof(set), &set) == 0,
           "Could not set CPU affinity.");
  }
}

void Thread::create() {
  if (m_created) {
    return;
  }
  ENSURE(m_thread == 0, "Pthread already created.");

  setSched();
  setAffinity();

  // commit pthread attributes
  ENSURE(m_system->pthread->pthread_attr_setinheritsched(&m_pthread_attr, PTHREAD_EXPLICIT_SCHED) ==
             0,
         "Could not set pthread attributes.");

  // lock all already mapped pages
  // don't use MCL_FUTURE cause then even NRT pages will be locked in future
  if (m_type == Type::RT) {
    ENSURE(m_system->mman->mlockall(MCL_CURRENT) == 0, "Could not lock pages.");
  }

  // create pthread
  m_is_running = true;
  ENSURE(m_system->pthread->pthread_create(&m_thread, &m_pthread_attr, &Thread::threadRun, this) ==
             0,
         "Could not create pthread.");

  // set name
  const char* name = m_name.substr(0, 15).c_str();
  ENSURE(m_system->pthread->pthread_setname_np(m_thread, name) == 0, "Could not set thread name.");

  // clear pthread attributes
  ENSURE(m_system->pthread->pthread_attr_destroy(&m_pthread_attr) == 0,
         "Could not destroy attribute.");

  // lock all pages afterwards cause now thread is created and uses new pages
  if (m_type == Type::RT) {
    ENSURE(m_system->mman->mlockall(MCL_CURRENT) == 0, "Could not lock pages.");
  }

  m_created = true;
  // TODO(sk) check thread properties as post condition
}

void Thread::cancel() {
  if (m_created) {
    if (m_thread != 0) {
      ENSURE(m_system->pthread->pthread_cancel(m_thread) == 0, "Could not cancel thread.");
    }
    m_created = false;
  }
}

void Thread::wake() {
  std::unique_lock<std::mutex> lock(m_prio_mutex);
  if (!m_got_wake_up) {
    m_got_wake_up = true;
    lock.unlock();
    m_wake_up_cond_var.notify_one();
    // TODO(sk) wake up sleeping non rt thread too
  }
}

void* Thread::threadRun(void* thread) {
  static_cast<Thread*>(thread)->run();
  return thread;
}

void Thread::run() {
  auto tick = std::chrono::steady_clock::now();
  auto next_tick = tick;
  while (m_is_running) {
    m_update();
    std::unique_lock<std::mutex> lock(m_prio_mutex);
    if (m_is_running && !m_got_wake_up) {
      if (m_period > 0us) {
        next_tick = tick + m_period;  // get relative sleep time
        while (!m_got_wake_up) {
          if (m_wake_up_cond_var.wait_until(lock, next_tick) == std::cv_status::timeout) {
            m_got_wake_up = true;
          }
        }
        tick = next_tick;
      } else {
        m_wake_up_cond_var.wait(lock, [&] { return m_got_wake_up.load(); });
      }
      m_got_wake_up = false;  // clear for next wait
    }
  }
}

void Thread::stop() {
  m_is_running = false;
  wake();
}

void Thread::join() {
  if (!m_is_running && m_thread != 0) {
    ENSURE(m_system->pthread->pthread_join(m_thread, nullptr) == 0, "Could not join thread.");
    m_created = false;
    m_thread = 0;
  }
}

}  // namespace fdl
