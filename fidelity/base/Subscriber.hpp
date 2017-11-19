#pragma once

#include <contract/contract_assert.hpp>

#include <boost/lockfree/spsc_queue.hpp>

#include <functional>
#include <string>

#include "Loop.hpp"
#include "PrioMutex.hpp"

namespace fdl {

template <typename MessageT>
class ISubscriber {
 public:
  virtual ~ISubscriber() = default;

  virtual const std::string& getName() const = 0;

  virtual bool write(const MessageT& data) = 0;
};

/**
 * Subscribing part of communication between running loops.
 * Subscriber receives data updates from publisher.
 * Data access is thread safe and lock free.
 * @tparam MessageT Type of pubsub message.
 */
template <typename MessageT>
class Subscriber : public ISubscriber<MessageT> {
 public:
  /**
   * Create named subscriber.
   * If a a loop reference is passed, this loop will be woken up on each data update.
   * @param name Name of subscriber.
   * @param capacity Capacity of communication connection buffer.
   * @param loop Loop which will be woken up on each data update if wanted.
   */
  Subscriber(const std::string& name, size_t capacity, ILoop* const loop = nullptr);

  ~Subscriber() override = default;

  /**
   * Get name of subscriber.
   * @return name Name of subscriber.
   */
  const std::string& getName() const override {
    return m_name;
  }

  /**
   * Write data to subscriber buffer.
   * Write is thread safe and lock free.
   * @param message Message data to be written.
   * @return true if data could be written.
   */
  bool write(const MessageT& message) override;

  /**
   * Read received data into an output variable.
   * Read is thread safe and lock free.
   * @param data Contains read data, if new data was received.
   * @return true if new data was successfully read, false if no data was received.
   */
  bool read(MessageT& data);

 private:
  /** Name of subscriber. */
  const std::string m_name{};

  /**
   * Single producer single consumer queue of boost is sufficient for publisher subscriber setup.
   * MessageT does not be trivial constructable or destructable which is an advantage to other boost
   * lock free queues. Queue configured with fixed size to avoid dynamic memory allocation during
   * writes.
   */
  boost::lockfree::spsc_queue<MessageT> m_queue{};

  /**
   * Loop to be woken up on each data write.
   * No shared ptr so far, cause then we would need to enable feature: enable_shared_from_this for
   * loops.
   */
  ILoop* const m_loop{};
};

template <typename MessageT>
Subscriber<MessageT>::Subscriber(const std::string& name, size_t capacity, ILoop* const loop)
    : m_name(name), m_queue(capacity), m_loop(loop) {
  EXPECT(!name.empty(), "Name must be empty.");
  EXPECT(capacity > 0, "Capacity must be greater 0.");
}

template <typename MessageT>
bool Subscriber<MessageT>::read(MessageT& message) {
  return m_queue.pop(message);
}

template <typename MessageT>
bool Subscriber<MessageT>::write(const MessageT& message) {
  bool written = m_queue.push(message);
  if (m_loop != nullptr) {
    m_loop->wake();
  }
  return written;
}

}  // namespace fdl
