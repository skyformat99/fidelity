#pragma once

#include <gmock/gmock.h>

#include <string>

#include "../Subscriber.hpp"

namespace fdl::test {

template <typename MessageT>
class SubscriberMock : public ISubscriber<MessageT> {
 public:
  virtual ~SubscriberMock() = default;

  MOCK_CONST_METHOD0(getName, const std::string&());
  MOCK_METHOD1_T(write, bool(const MessageT&));
};

}  // namespace fdl::test
