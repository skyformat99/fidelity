#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <string>

#include <cstddef>

#include "Thread.hpp"

namespace fdl {

namespace test::loop {
class BASE_LoopTest;
}  // namespace test::loop

class ILoop {
 public:
  virtual ~ILoop() = default;

  virtual void setPeriod(std::chrono::microseconds period) = 0;

  virtual void setStackSize(size_t size) = 0;

  virtual bool configure() = 0;

  virtual bool start() = 0;

  virtual void wake() = 0;

  virtual bool stop() = 0;

  virtual void cancel() = 0;
};

/**
 * A Loop represents the working task to be executed by the underlying thread.
 * Custom Loops need to be inherited from Loop base class and can be configured as RT (realtime) or
 * NON_RT (non realtime). The callback methods onConfigure(), onStart(), onRun() and onStop() can be
 * overwritten for initialization/deinitialization.
 *
 * If a Loop is configured with a period (setPeriodUs()), onRun() will be called periodically
 * with the configured period. If no period is set, the loop needs to be woken up with wake()
 * (e.g. when new data is available) to trigger onRun().
 */
class Loop : public ILoop {
 public:
  ~Loop() override;

  Loop(const Loop&) = delete;
  Loop(Loop&&) = delete;
  Loop& operator=(Loop&&) = delete;
  Loop& operator=(const Loop&) = delete;

  /**
   * Set period of the loop. onRun() will be called periodically.
   * @param period Loop period in microseconds.
   */
  void setPeriod(std::chrono::microseconds period) override;

  /**
   * Set stack size of underlying thread, if default is non enough.
   * @param size New stack size in Byte.
   */
  void setStackSize(size_t size) override;

  /**
   * Configure loop by calling onConfigure().
   * @return true on success.
   */
  bool configure() final;

  /**
   * Start loop by creating underlying thread and calling onStart().
   * @return true on success.
   */
  bool start() final;

  /**
   * Wake thread for executing onRun().
   */
  void wake() override;

  /**
   * Stop loop and wait for thread completion. Do clean up in onStop().
   * @return true on success.
   */
  bool stop() final;

  /**
   * Cancel thread immediately.
   */
  void cancel() final;

 protected:
  /**
   * Create and setup a Loop object.
   * @param name Name of loop which will become name of the thread. Names which are longer then
   *        15 characters will be cut in thread (pthread boundary).
   * @param type Type of loop (NON_RT, RT).
   * @param prio Priority of thread. 98 is highest, 1 lowest.
   * @param affinity The CPU which will be used for possible thread: -1 means no binding.
   */
  Loop(const std::string& name, Thread::Type type, int prio, int affinity);

  /**
   * Configuration method for custom loops.
   * @return true on success.
   */
  virtual bool onConfigure() {
    return true;
  }

  /**
   * Start method for custom loops.
   * @return true on success.
   */
  virtual bool onStart() {
    return true;
  }

  /**
   * Run method for custom loops.
   * This is the actual work to do, when loop owns CPU. The onRun() callback will be called
   * event triggered or periodically.
   */
  virtual void onRun() {}

  /**
   * Stop method for custom loops.
   * @return true on success.
   */
  virtual bool onStop() {
    return true;
  }

 private:
  friend class test::loop::BASE_LoopTest;

  /** Underlying thread dependency injection for tests.*/
  static std::shared_ptr<IThread> m_thread_di;

  /** Underlying thread of the loops. */
  std::shared_ptr<IThread> m_thread{};

  /** Name of underlying thread. */
  const std::string m_name{};

  /** Type of underlying thread. */
  const Thread::Type m_type{Thread::Type::NON_RT};

  /** Priority of underlying thread. */
  const int m_prio{-1};

  /** CPU affinity of underlying thread. */
  const int m_affinity{-1};

  /** Configuration state of loop. */
  std::atomic<bool> m_is_configured{false};

  /** Running state of loop. */
  std::atomic<bool> m_is_running{false};
};

/**
 * A loop preconfigured as realtime loop.
 * Only name, priority and CPI affinity needs to be set.
 */
class RTLoop : public Loop {
 public:
  /**
   * Constructor, which configures loop as realtime loop.
   * @param name Name of loop.
   * @param prio Priority of loop.
   * @param affinity Affinity of loop. Default -1 won't set affinity.
   */
  explicit RTLoop(const std::string& name, int prio = 50, int affinity = -1)
      : Loop(name, Thread::Type::RT, prio, affinity) {}
};

/**
 * A loop preconfigured as non realtime loop.
 */
class NonRTLoop : public Loop {
 public:
  /**
   * Constructor, which configures loop as non realtime loop.
   * @param name Name of loop.
   * @param affinity Affinity of loop. Default -1 won't set affinity.
   */
  explicit NonRTLoop(const std::string& name, int affinity = -1)
      : Loop(name, Thread::Type::NON_RT, 0, affinity) {}
};

}  // namespace fdl
