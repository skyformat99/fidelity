#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <contract/contract_assert.hpp>

#include <chrono>
#include <memory>

#include "Definitions.hpp"
#include "ThreadMock.hpp"

#include "../Loop.hpp"

using namespace std::chrono_literals;

namespace t = testing;

namespace fdl {
class IThread;
}

namespace fdl::test::loop {

class TestRTLoop : public RTLoop {
 public:
  TestRTLoop() : RTLoop("rt_loop"){};
  virtual ~TestRTLoop() = default;

  MOCK_METHOD0(onConfigure, bool());
  MOCK_METHOD0(onStart, bool());
  MOCK_METHOD0(onRun, void());
  MOCK_METHOD0(onStop, bool());
};

class TestNonRTLoop : public NonRTLoop {
 public:
  TestNonRTLoop() : NonRTLoop("non_rt_loop"){};
  virtual ~TestNonRTLoop() = default;

  MOCK_METHOD0(onConfigure, bool());
};

class BASE_LoopTest : public t::Test {
 public:
  virtual void TearDown() {
    injectThread(nullptr);
  }

  void injectThread(std::shared_ptr<IThread> thread) {
    Loop::m_thread_di = thread;
  }

  void callUpdate(Loop& loop) {
    loop.onRun();
  }
};

DESCRIBE_F(BASE_LoopTest, constructor, should_check_precondidtions) {
  // expect non empty name
  EXPECT_THROW(RTLoop(""), std::experimental::contract_violation_error);
  EXPECT_THROW(NonRTLoop(""), std::experimental::contract_violation_error);
}

DESCRIBE_F(BASE_LoopTest, setPeriod, should_set_loop_period) {
  auto thread_mock = std::make_shared<ThreadMock>();
  injectThread(thread_mock);

  RTLoop loop("rt_loop");
  EXPECT_TRUE(loop.configure());
  EXPECT_CALL(*thread_mock, setPeriod(1000us));
  loop.setPeriod(1000us);
}

DESCRIBE_F(BASE_LoopTest, setStackSize, should_set_stack_size) {
  auto thread_mock = std::make_shared<ThreadMock>();
  injectThread(thread_mock);

  RTLoop loop("rt_loop");
  EXPECT_TRUE(loop.configure());
  EXPECT_CALL(*thread_mock, setStackSize(1024));
  loop.setStackSize(1024);
}

DESCRIBE_F(BASE_LoopTest, callbacks, should_return_true, if_not_overwritten) {
  auto thread_mock = std::make_shared<ThreadMock>();
  injectThread(thread_mock);

  RTLoop loop("rt_loop");
  EXPECT_CALL(*thread_mock, create());
  EXPECT_CALL(*thread_mock, stop());
  EXPECT_CALL(*thread_mock, join());
  EXPECT_TRUE(loop.configure());
  EXPECT_TRUE(loop.start());
  callUpdate(loop);
  EXPECT_TRUE(loop.stop());
}

DESCRIBE_F(BASE_LoopTest, configure, should_call_onConfigure) {
  auto thread_mock = std::make_shared<ThreadMock>();
  injectThread(thread_mock);

  TestRTLoop loop;
  EXPECT_CALL(loop, onConfigure()).WillOnce(t::Return(true));
  EXPECT_TRUE(loop.configure());

  // should be callable only once
  EXPECT_THROW(loop.configure(), std::experimental::contract_violation_error);

  TestRTLoop loop_2;
  EXPECT_CALL(loop_2, onConfigure()).WillOnce(t::Return(false));
  EXPECT_FALSE(loop_2.configure());

  TestNonRTLoop loop_3;
  EXPECT_CALL(loop_3, onConfigure()).WillOnce(t::Return(false));
  EXPECT_FALSE(loop_3.configure());
}

DESCRIBE_F(BASE_LoopTest, start, should_call_onStart) {
  auto thread_mock = std::make_shared<ThreadMock>();
  injectThread(thread_mock);

  TestRTLoop loop;

  // should not be callable before configuration
  EXPECT_THROW(loop.start(), std::experimental::contract_violation_error);

  EXPECT_CALL(loop, onConfigure()).WillOnce(t::Return(true));
  EXPECT_TRUE(loop.configure());

  EXPECT_CALL(loop, onStart()).WillOnce(t::Return(true));
  EXPECT_CALL(*thread_mock, create());
  EXPECT_TRUE(loop.start());

  // should be callable only once
  EXPECT_THROW(loop.start(), std::experimental::contract_violation_error);

  TestRTLoop loop_2;
  EXPECT_CALL(loop_2, onConfigure()).WillOnce(t::Return(true));
  EXPECT_TRUE(loop_2.configure());

  EXPECT_CALL(loop_2, onStart()).WillOnce(t::Return(false));
  EXPECT_FALSE(loop_2.start());

  // called by destructor
  EXPECT_CALL(*thread_mock, stop());
  EXPECT_CALL(*thread_mock, join());
}

DESCRIBE_F(BASE_LoopTest, wake, should_wake_thread) {
  auto thread_mock = std::make_shared<ThreadMock>();
  injectThread(thread_mock);

  TestRTLoop loop;

  // should not be callable before started
  EXPECT_THROW(loop.wake(), std::experimental::contract_violation_error);

  EXPECT_CALL(loop, onConfigure()).WillOnce(t::Return(true));
  EXPECT_TRUE(loop.configure());

  // should not be callable before started
  EXPECT_THROW(loop.wake(), std::experimental::contract_violation_error);

  EXPECT_CALL(loop, onStart()).WillOnce(t::Return(true));
  EXPECT_CALL(*thread_mock, create());
  EXPECT_TRUE(loop.start());

  EXPECT_CALL(*thread_mock, wake());

  loop.wake();

  // called by destructor
  EXPECT_CALL(*thread_mock, stop());
  EXPECT_CALL(*thread_mock, join());
}

DESCRIBE_F(BASE_LoopTest, update, should_call_onRun) {
  auto thread_mock = std::make_shared<ThreadMock>();
  injectThread(thread_mock);

  TestRTLoop loop;
  EXPECT_CALL(loop, onConfigure()).WillOnce(t::Return(true));
  EXPECT_TRUE(loop.configure());

  EXPECT_CALL(loop, onStart()).WillOnce(t::Return(true));
  EXPECT_CALL(*thread_mock, create());
  EXPECT_TRUE(loop.start());

  EXPECT_CALL(loop, onRun());

  callUpdate(loop);

  // called by destructor
  EXPECT_CALL(*thread_mock, stop());
  EXPECT_CALL(*thread_mock, join());
}

DESCRIBE_F(BASE_LoopTest, stop, should_stop_thread) {
  auto thread_mock = std::make_shared<ThreadMock>();
  injectThread(thread_mock);

  TestRTLoop loop;
  EXPECT_CALL(loop, onConfigure()).WillOnce(t::Return(true));
  EXPECT_TRUE(loop.configure());

  EXPECT_CALL(loop, onStart()).WillOnce(t::Return(true));
  EXPECT_CALL(*thread_mock, create());
  EXPECT_TRUE(loop.start());

  EXPECT_CALL(*thread_mock, stop());
  EXPECT_CALL(*thread_mock, join());
  EXPECT_CALL(loop, onStop()).WillOnce(t::Return(true));

  loop.stop();
}

DESCRIBE_F(BASE_LoopTest, stop, should_call_onStop) {
  auto thread_mock = std::make_shared<ThreadMock>();
  injectThread(thread_mock);

  TestRTLoop loop;

  // should not be callable before started
  EXPECT_THROW(loop.stop(), std::experimental::contract_violation_error);

  EXPECT_CALL(loop, onConfigure()).WillOnce(t::Return(true));
  EXPECT_TRUE(loop.configure());

  // should not be callable before started
  EXPECT_THROW(loop.stop(), std::experimental::contract_violation_error);

  EXPECT_CALL(loop, onStart()).WillOnce(t::Return(true));
  EXPECT_CALL(*thread_mock, create());
  EXPECT_TRUE(loop.start());

  EXPECT_CALL(loop, onStop()).WillOnce(t::Return(true));
  EXPECT_CALL(*thread_mock, stop());
  EXPECT_CALL(*thread_mock, join());
  EXPECT_TRUE(loop.stop());

  // should be callable only once
  EXPECT_THROW(loop.stop(), std::experimental::contract_violation_error);

  TestRTLoop loop_2;
  EXPECT_CALL(loop_2, onConfigure()).WillOnce(t::Return(true));
  EXPECT_TRUE(loop_2.configure());

  EXPECT_CALL(loop_2, onStart()).WillOnce(t::Return(true));
  EXPECT_CALL(*thread_mock, create());
  EXPECT_TRUE(loop_2.start());

  EXPECT_CALL(loop_2, onStop()).WillOnce(t::Return(false));
  EXPECT_CALL(*thread_mock, stop());
  EXPECT_CALL(*thread_mock, join());
  EXPECT_FALSE(loop_2.stop());
}

DESCRIBE_F(BASE_LoopTest, cancel, should_cancel_thread) {
  auto thread_mock = std::make_shared<ThreadMock>();
  injectThread(thread_mock);

  TestRTLoop loop;
  EXPECT_CALL(loop, onConfigure()).WillOnce(t::Return(true));
  EXPECT_TRUE(loop.configure());
  EXPECT_CALL(*thread_mock, cancel());
  loop.cancel();
}

DESCRIBE_F(BASE_LoopTest, destructor, should_join_thread) {
  auto thread_mock = std::make_shared<ThreadMock>();
  injectThread(thread_mock);

  bool done = false;
  {
    TestRTLoop loop;
    EXPECT_CALL(loop, onConfigure()).WillOnce(t::Return(true));
    EXPECT_TRUE(loop.configure());

    EXPECT_CALL(loop, onStart()).WillOnce(t::Return(true));
    EXPECT_CALL(*thread_mock, create());
    EXPECT_TRUE(loop.start());

    EXPECT_CALL(*thread_mock, stop());
    EXPECT_CALL(*thread_mock, join()).WillOnce(t::Invoke([&done] { done = true; }));
  }
  // check if destructor was called yet
  EXPECT_TRUE(done);
}

}  // namespace fdl::test::loop
