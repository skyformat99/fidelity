#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>

#include <fidelity/base/test/Definitions.hpp>

#include <fidelity/base/Publisher.hpp>
#include <fidelity/base/Subscriber.hpp>
#include <fidelity/base/Loop.hpp>

using namespace std::chrono_literals;

namespace fdl::test::pubsub_scenario {

// pubsub message type
struct Message {
  char name{0};
  bool valid{false};
  int count{0};
  float value{0.0};
  double result{0.0};

  bool operator==(const Message& other) const {
    bool equal = true;
    equal &= name == other.name;
    equal &= valid == other.valid;
    equal &= count == other.count;
    equal &= value == other.value;
    equal &= result == other.result;
    return equal;
  }
};

DESCRIBE(BASE_PubSubScenario, subscriber, should_receive_published_messages) {
  Publisher<Message> publisher("publisher");
  auto subscriber = std::make_shared<Subscriber<Message>>("subscriber", 1);
  const Message message{'x', false, 1, 0.5f, 0.5};

  EXPECT_TRUE(publisher.subscribe(subscriber));
  EXPECT_TRUE(publisher.write(message));

  // should fail on full buffer
  EXPECT_FALSE(publisher.write(message));

  Message read_message;
  EXPECT_TRUE(subscriber->read(read_message));
  EXPECT_EQ(message, read_message);

  // should fail on empty buffer
  EXPECT_FALSE(subscriber->read(read_message));
}

DESCRIBE(BASE_PubSubScenario, all_subscribers, should_receive_published_messages) {
  Publisher<Message> publisher("publisher");
  auto subscriber_1 = std::make_shared<Subscriber<Message>>("subscriber_1", 1);
  auto subscriber_2 = std::make_shared<Subscriber<Message>>("subscriber_2", 1);
  const Message message{'x', false, 1, 0.5f, 0.5};

  EXPECT_TRUE(publisher.subscribe(subscriber_1));
  EXPECT_TRUE(publisher.subscribe(subscriber_2));
  EXPECT_TRUE(publisher.write(message));

  Message read_message;
  EXPECT_TRUE(subscriber_1->read(read_message));
  EXPECT_EQ(message, read_message);

  EXPECT_TRUE(subscriber_2->read(read_message));
  EXPECT_EQ(message, read_message);
}

// std::atomic<bool> send_done{false};
// std::atomic<bool> receive_done{false};
//

class Sender : public RTLoop {
 public:
  Sender(Message message) : RTLoop("sender"), m_message(message) {}

  std::shared_ptr<Publisher<Message>> getPublisher() const {
    return m_publisher;
  }

  int getCount() const {
    return m_count;
  }

  Message getMessage() const {
    return m_message;
  }

  void onRun() override {
    Message message = m_message;
    m_publisher->write(message);
    m_count++;
  }

 private:
  std::shared_ptr<Publisher<Message>> m_publisher{
      std::make_shared<Publisher<Message>>("publisher")};

  int m_count{0};
  Message m_message{};
};

class Receiver : public RTLoop {
 public:
  Receiver() : RTLoop("receiver") {}

  std::shared_ptr<Subscriber<Message>> getSubscriber() const {
    return m_subscriber;
  }

  int getCount() const {
    return m_count;
  }

  Message getMessage() const {
    return m_message;
  }

  void onRun() override {
    while (m_subscriber->read(m_message)) {
      m_count++;
    };
  }

 private:
  std::shared_ptr<Subscriber<Message>> m_subscriber{
      std::make_shared<Subscriber<Message>>("subscriber", 2)};

  int m_count{0};
  Message m_message{};
};

DESCRIBE(BASE_PubSubScenario, publishing_and_receiving_messages, should_execute) {
  Sender sender({'x', false, 1, 0.5f, 0.5});
  Receiver receiver;

  auto publisher = sender.getPublisher();
  auto subscriber = receiver.getSubscriber();
  EXPECT_TRUE(publisher->subscribe(subscriber));

  EXPECT_TRUE(sender.configure());
  EXPECT_TRUE(sender.start());
  EXPECT_TRUE(receiver.configure());
  EXPECT_TRUE(receiver.start());

  for (int i = 0; i < 10; i++) {
    sender.wake();
    receiver.wake();
    std::this_thread::sleep_for(1ms);
  }

  EXPECT_EQ(11, sender.getCount());
  EXPECT_EQ(sender.getCount(), receiver.getCount());
  EXPECT_EQ(sender.getMessage(), receiver.getMessage());

  sender.stop();
  receiver.stop();
}

}  // namespace fdl::test::pubsub_scenario
