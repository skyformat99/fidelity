#pragma once

#include <gmock/gmock.h>

#include "../Thread.hpp"

namespace fdl::test {

class ThreadMock : public IThread {
 public:
  ~ThreadMock() override = default;

  MOCK_METHOD1(setPeriod, void(std::chrono::microseconds));
  MOCK_METHOD1(setStackSize, void(size_t));
  MOCK_METHOD0(create, void());
  MOCK_METHOD0(cancel, void());
  MOCK_METHOD0(wake, void());
  MOCK_METHOD0(stop, void());
  MOCK_METHOD0(join, void());
};

}  // namespace fdl::test
