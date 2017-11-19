#include <sstream>
#include <stdexcept>
//#include <stdio.h>

#include <contract/contract_assert.hpp>
#include <gtest/gtest.h>

#include "tap.h"

void throw_on_contract_violation(const std::experimental::contract_violation_info& info) {
  throw std::experimental::contract_violation_error(info);
}

GTEST_API_ int main(int argc, char** argv) {

  std::experimental::set_handle_contract_violation(throw_on_contract_violation);

  // printf("Running main() from gtest-tap-main.cpp\n");
  testing::InitGoogleTest(&argc, argv);

  testing::TestEventListeners& listeners = testing::UnitTest::GetInstance()->listeners();

  // Delete the default listener
  // delete listeners.Release(listeners.default_result_printer());
  listeners.Append(new tap::TapListener());

  return RUN_ALL_TESTS();
}
