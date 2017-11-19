#include "Loop.hpp"

#include <contract/contract_assert.hpp>

#include <memory>
#include <string>

#include "Thread.hpp"

namespace fdl {

std::shared_ptr<IThread> Loop::m_thread_di{nullptr};

Loop::Loop(const std::string& name, Thread::Type type, int prio, int affinity)
    : m_name(name), m_type(type), m_prio(prio), m_affinity(affinity) {
  EXPECT(!name.empty(), "Loop needs to be named.");
}

Loop::~Loop() {
  if (m_is_running) {
    stop();
  }
}

void Loop::setPeriod(std::chrono::microseconds period) {
  EXPECT(m_is_configured, "Loop not configured: call setPeriod in onConfigure.");
  m_thread->setPeriod(period);
}

void Loop::setStackSize(size_t size) {
  EXPECT(m_is_configured, "Loop not configured: call setStackSize in onConfigure.");
  m_thread->setStackSize(size);
}

bool Loop::configure() {
  ENSURE(!m_is_configured, "Loop already configured.");

  if (Loop::m_thread_di != nullptr) {
    m_thread = Loop::m_thread_di;
  } else {
    m_thread = std::make_shared<Thread>(m_name, m_type, m_prio, m_affinity, [this] { onRun(); });
    ENSURE(m_thread != nullptr);
  }

  m_is_configured = true;
  m_is_configured = onConfigure();
  return m_is_configured;
}

bool Loop::start() {
  ENSURE(m_is_configured, "Loop not configured, call configure() first.");
  ENSURE(!m_is_running, "Loop already running.");

  if (onStart()) {
    m_thread->create();
    m_is_running = true;
    return true;
  }
  return false;
}

void Loop::wake() {
  ENSURE(m_is_running, "Loop not running, call start() first.");
  m_thread->wake();
}

bool Loop::stop() {
  ENSURE(m_is_running, "Loop not running.");

  m_is_running = false;

  m_thread->stop();
  m_thread->join();

  return onStop();
}

void Loop::cancel() {
  if (m_is_configured) {
    m_thread->cancel();
  }
}

}  // namespace fdl
