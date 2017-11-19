#pragma once

#include <contract/contract_assert.hpp>

#include <list>
#include <memory>
#include <string>

namespace fdl {

template <typename MessageT>
class ISubscriber;

/**
 * Publishing part of communication between running loops.
 * For publishing data thread safe and lock free.
 */
template <typename MessageT>
class Publisher {
 public:
  /**
   * Create publisher.
   * @param name Name of publisher.
   */
  explicit Publisher(const std::string& name);

  /**
   * Get name of publisher.
   * @return name Name of publisher.
   */
  const std::string& getName() const {
    return m_name;
  }

  /**
   * Register new subscriber to publisher.
   * Each subscriber will receive data updates after successful subscription.
   * Name of new subscriber needs to be unique for publisher.
   * @tparam MessageT Type of pubsub message.
   * @param subscriber Subscriber which will be added.
   * @return true on success, false otherwise.
   */
  bool subscribe(std::shared_ptr<ISubscriber<MessageT>> subscriber);

  /**
   * Publish data to subscriber.
   * @param message Message data to publish.
   * @return true if data could be written.
   */
  bool write(const MessageT& message);

 private:
  /** Name of publisher. */
  const std::string m_name{};

  /** List of all registered subscribers. */
  std::list<std::shared_ptr<ISubscriber<MessageT>>> m_subscriber_list{};
};

template <typename MessageT>
Publisher<MessageT>::Publisher(const std::string& name) : m_name(name) {
  EXPECT(!name.empty(), "Publisher needs to be named.");
}

template <typename MessageT>
bool Publisher<MessageT>::subscribe(std::shared_ptr<ISubscriber<MessageT>> subscriber) {
  if (subscriber == nullptr) {
    return false;
  }
  if (subscriber->getName().empty()) {
    return false;
  }

  auto found_subscriber =
      std::find_if(m_subscriber_list.begin(), m_subscriber_list.end(),
                   [&](const std::shared_ptr<ISubscriber<MessageT>>& subscriber_it) {
                     return subscriber_it->getName() == subscriber->getName();
                   });

  // subscriber already added
  if (found_subscriber != m_subscriber_list.end()) {
    return false;
  }
  // add to subscriber list
  m_subscriber_list.push_back(subscriber);

  return true;
}

template <typename MessageT>
bool Publisher<MessageT>::write(const MessageT& message) {
  bool success = true;
  for (auto& subscriber : m_subscriber_list) {
    if (subscriber != nullptr) {  // TODO(sk) ensure
      success &= subscriber->write(message);
    }
  }
  return success;
}

}  // namespace fdl
