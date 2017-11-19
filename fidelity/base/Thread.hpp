#pragma once

#include <pthread.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>

#include "PrioMutex.hpp"

namespace fdl {

struct SystemAdapter;

namespace test::thread {
class BASE_ThreadTest;
}  // namespace test::thread

namespace test::pthread_scenario {
class BASE_PthreadScenario;
}  // namespace test::pthread_scenario

/** Thread class interface. */
class IThread {
 public:
  virtual ~IThread() = default;

  /** @copydoc Thread::setPeriod */
  virtual void setPeriod(std::chrono::microseconds period) = 0;

  /** @copydoc Thread::setStackSize */
  virtual void setStackSize(size_t size) = 0;

  /** @copydoc Thread::create */
  virtual void create() = 0;

  /** @copydoc Thread::cancel */
  virtual void cancel() = 0;

  /** @copydoc Thread::wake */
  virtual void wake() = 0;

  /** @copydoc Thread::stop */
  virtual void stop() = 0;

  /** @copydoc Thread::join */
  virtual void join() = 0;
};

/**
 * This class encapsulates POSIX thread (pthread) management.
 * Creations of realtime and non realtime threads are provided. The name, priority and CPU
 * affinity can be set of each thread. On Thread::run() the given update() function is called.
 *
 * Threads have to be configured either as event triggered or periodic threads. Event triggered
 * threads will be woken up by calling wake(). Periodic threads always sleep a configured duration.
 *
 * @note Can't use c++11 std::threads because of missing interfaces to set stack size, scheduler,
 * priority, stack size and affinity.
 */
class Thread : public IThread {
 public:
  /** The type of thread. */
  enum class Type {
    /** realtime pthread (SCHED_FIFO). */
    RT,
    /** Non realtime pthread (SCHED_OTHER). */
    NON_RT
  };

  /**
   * Create thread.
   * @param name Name of thread (only 15 characters can be passed to pthread).
   * @param type Type of thread (realtime/non realtime).
   * @param prio Priority of thread for realtime threads (98 highest, 1 lowest).
   * @param affinity CPU which will be used for binding thread (-1 means no binding).
   * @param update Update function triggered by thread run().
   */
  Thread(const std::string& name, Thread::Type type, int prio, int affinity,
         std::function<void()> update);

  /**
   * Set period of thread.
   * @param period Period in microseconds.
   */
  void setPeriod(std::chrono::microseconds period) override;

  /**
   * Set stack size of thread.
   * Wanted stack size on top of PTHREAD_STACK_MIN. The actual stack size may be greater than the
   * requested size, because previous joined threads are reused if the requested stack size fits.
   * @param size New stack size in byte.
   */
  void setStackSize(size_t size) final;

  /**
   * Get creation state of thread.
   * @return true if pthread was successfully created, otherwise false.
   */
  bool get_created() const {
    return m_created;
  }

  /** Set up and create pthread. */
  void create() override;

  /** Cancel thread. */
  void cancel() override;

  /** Wake thread, which will end in calling run(). */
  void wake() override;

  /**
   * Thread run method.
   * If thread configured as periodic thread, thread will sleep after run().
   * If thread configured as event triggered, thread will wait for wake up after run().
   */
  static void* threadRun(void* thread);

  /** Stop thread, which will end in finishing run(). */
  void stop() override;

  /** Join and cleanup thread. */
  void join() override;

 public:
  /** Default stack size of thread in byte. */
  static constexpr size_t DEFAULT_STACK_SIZE = 2048 * 1024;

 private:
  friend class test::thread::BASE_ThreadTest;
  friend class test::pthread_scenario::BASE_PthreadScenario;

  /** Set CPU affinity property of pthread attribute for thread creation. */
  void setAffinity();

  /** Set scheduler property of pthread attribute for thread creation. */
  void setSched();

  void run();

 private:
  /** System adapter class dependency injection for tests. */
  static std::shared_ptr<SystemAdapter> m_system_di;

  /** System adapter class for library calls. */
  std::shared_ptr<SystemAdapter> m_system{};

  /** Name of thread. */
  const std::string m_name{};

  /** Type of thread. */
  const Thread::Type m_type{Thread::Type::NON_RT};

  /** Priority of thread. */
  const int m_prio{-1};

  /** CPU affinity of thread. */
  const int m_affinity{-1};

  /** Period of thread in us. */
  std::chrono::microseconds m_period{0};

  /** Event triggering state of thread. */
  bool m_is_event_triggered{true};

  /** Maximum stack size of thread. */
  size_t m_max_stack_size{0};

  /** The underlying pthread. */
  pthread_t m_thread{};

  /** Attributes of to be created pthread. */
  pthread_attr_t m_pthread_attr{};

  /** Creation state of thread. */
  bool m_created{false};

  /** Update functor to be called from thread. */
  std::function<void()> m_update;

  /**
   * Mutex for synchronizing thread wake up.
   * @see http://en.cppreference.com/w/cpp/thread/condition_variable
   */
  PrioMutex m_prio_mutex{};

  /** Condition variable for waiting for wake up. */
  std::condition_variable m_wake_up_cond_var{};

  /** Wake up state of the tread, to be checked before wait. */
  std::atomic<bool> m_got_wake_up{false};

  /** Running state of thread. */
  std::atomic<bool> m_is_running{false};
};

}  // namespace fdl
