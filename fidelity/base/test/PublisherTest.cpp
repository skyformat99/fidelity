#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <contract/contract_assert.hpp>

#include <memory>
#include <string>

#include "Definitions.hpp"
#include "SubscriberMock.hpp"

#include "../Publisher.hpp"

namespace t = testing;

namespace fdl::test::publisher {

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

class BASE_PublisherTest : public t::Test {};

DESCRIBE_F(BASE_PublisherTest, constructor, should_check_precondidtions) {
  // expect non empty name
  EXPECT_THROW(Publisher<Message>(""), std::experimental::contract_violation_error);
}

DESCRIBE_F(BASE_PublisherTest, getName, should_return_name) {
  Publisher<Message> publisher("publisher");
  EXPECT_EQ(std::string("publisher"), publisher.getName());
}

DESCRIBE_F(BASE_PublisherTest, subscribe, should_return_true, if_subscriber_is_valid) {
  Publisher<Message> publisher("publisher");

  // check nullptr
  EXPECT_FALSE(publisher.subscribe(nullptr));

  std::string subscriber_name = "subscriber";
  auto mock = std::make_shared<SubscriberMock<Message>>();
  EXPECT_CALL(*mock, getName()).WillOnce(t::ReturnRef(subscriber_name));
  EXPECT_TRUE(publisher.subscribe(mock));
}

DESCRIBE_F(BASE_PublisherTest, subscribe, should_return_false, if_subscriber_name_is_empty) {
  Publisher<Message> publisher("publisher");

  std::string subscriber_name = "";
  auto mock = std::make_shared<SubscriberMock<Message>>();
  EXPECT_CALL(*mock, getName()).WillOnce(t::ReturnRef(subscriber_name));
  EXPECT_FALSE(publisher.subscribe(mock));
}

DESCRIBE_F(BASE_PublisherTest, subscribe, should_return_false,
           if_subscriber_with_same_name_is_already_subscribed) {
  Publisher<Message> publisher("publisher");

  std::string subscriber_name = "subscriber";
  auto mock = std::make_shared<SubscriberMock<Message>>();
  EXPECT_CALL(*mock, getName()).WillRepeatedly(t::ReturnRef(subscriber_name));
  EXPECT_TRUE(publisher.subscribe(mock));
  EXPECT_FALSE(publisher.subscribe(mock));
}

DESCRIBE_F(BASE_PublisherTest, write, should_publish_message_to_subscribers) {
  Publisher<Message> publisher("publisher");

  std::string subscriber_name_1 = "subscriber_1";
  std::string subscriber_name_2 = "subscriber_2";
  auto mock_1 = std::make_shared<SubscriberMock<Message>>();
  auto mock_2 = std::make_shared<SubscriberMock<Message>>();
  EXPECT_CALL(*mock_1, getName()).WillRepeatedly(t::ReturnRef(subscriber_name_1));
  EXPECT_CALL(*mock_2, getName()).WillRepeatedly(t::ReturnRef(subscriber_name_2));
  EXPECT_TRUE(publisher.subscribe(mock_1));
  EXPECT_TRUE(publisher.subscribe(mock_2));

  Message message;
  EXPECT_CALL(*mock_1, write(message)).WillOnce(t::Return(true));
  EXPECT_CALL(*mock_2, write(message)).WillOnce(t::Return(true));
  EXPECT_TRUE(publisher.write(message));
}

}  // namespace fdl::test::publisher
