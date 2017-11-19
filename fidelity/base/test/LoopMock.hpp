#pragma once

#include <gmock/gmock.h>

#include "../Loop.hpp"

namespace fdl::test {

class LoopMock : public ILoop {
 public:
  virtual ~LoopMock() = default;

  MOCK_METHOD1(setPeriod, void(std::chrono::microseconds));
  MOCK_METHOD1(setStackSize, void(size_t));
  MOCK_METHOD0(configure, bool());
  MOCK_METHOD0(start, bool());
  MOCK_METHOD0(wake, void());
  MOCK_METHOD0(stop, bool());
  MOCK_METHOD0(cancel, void());
};

}  // namespace fdl::test
