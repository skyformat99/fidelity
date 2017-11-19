#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <contract/contract_assert.hpp>

#include <string>

#include "Definitions.hpp"
#include "LoopMock.hpp"

#include "../Subscriber.hpp"

namespace t = testing;

namespace fdl::test::subscriber {

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

class BASE_SubscriberTest : public t::Test {};

DESCRIBE_F(BASE_SubscriberTest, constructor, should_check_precondidtions) {
  // expect non empty name
  EXPECT_THROW(Subscriber<Message>("", 1), std::experimental::contract_violation_error);
  // expect capacity greater than 0
  EXPECT_THROW(Subscriber<Message>("subscriber", 0), std::experimental::contract_violation_error);
}

DESCRIBE_F(BASE_SubscriberTest, getName, should_return_name) {
  Subscriber<Message> subscriber("subscriber", 1);
  EXPECT_EQ(std::string("subscriber"), subscriber.getName());
}

DESCRIBE_F(BASE_SubscriberTest, write, should_wake_loop) {
  LoopMock mock;
  Subscriber<Message> subscriber("subscriber", 1, &mock);

  Message message;
  EXPECT_CALL(mock, wake());
  EXPECT_TRUE(subscriber.write(message));

  // without binding a loop to wake
  Subscriber<Message> subscriber_2("subscriber", 1, nullptr);
  EXPECT_TRUE(subscriber_2.write(message));
}

DESCRIBE_F(BASE_SubscriberTest, write, should_return_false, if_queue_is_full) {
  LoopMock mock;
  Subscriber<Message> subscriber("subscriber", 1, &mock);

  Message message;
  EXPECT_CALL(mock, wake()).Times(2);
  EXPECT_TRUE(subscriber.write(message));
  EXPECT_FALSE(subscriber.write(message));
}

DESCRIBE_F(BASE_SubscriberTest, read, should_receive_the_message) {
  LoopMock mock;
  Subscriber<Message> subscriber("subscriber", 1, &mock);

  Message message{'x', false, 1, 0.5f, 0.5};
  Message read_message;
  EXPECT_CALL(mock, wake());
  EXPECT_TRUE(subscriber.write(message));
  EXPECT_TRUE(subscriber.read(read_message));
  EXPECT_EQ(message, read_message);
}

DESCRIBE_F(BASE_SubscriberTest, read, should_return_false, if_queue_is_empty) {
  Subscriber<Message> subscriber("subscriber", 1);
  Message message;
  EXPECT_FALSE(subscriber.read(message));
}

}  // namespace fdl::test::subscriber
