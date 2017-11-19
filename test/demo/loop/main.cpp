#include <fidelity/base/Loop.hpp>

#include <contract/contract_assert.hpp>

#include <chrono>
#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <thread>

namespace fs = std::experimental::filesystem;

using namespace std::chrono_literals;

void throw_on_contract_violation(const std::experimental::contract_violation_info& info) {
  throw std::experimental::contract_violation_error(info);
}

void usage(const std::string& cmd) {
  std::cout << "Realtime loop demo" << std::endl;
  std::cout << "Usage: " << fs::path(cmd).filename().c_str() << " <duration in s>" << std::endl;
}

class MyLoop : public fdl::RTLoop {
 public:
  MyLoop() : RTLoop("rt_loop") {}

  bool onConfigure() override {
    setPeriod(1000us);
    std::cout << " configured..." << std::endl;
    return true;
  }

  bool onStart() override {
    std::cout << " started..." << std::endl;
    return true;
  }

  void onRun() override {
    m_count++;
  }

  bool onStop() override {
    std::cout << " stopped..." << std::endl;
    return true;
  }

  std::size_t getCount() const {
    return m_count;
  }

 private:
  std::size_t m_count{0};
};

int main(int argc, char* argv[]) {
  std::experimental::set_handle_contract_violation(throw_on_contract_violation);
  std::string command = argv[0];  // NOLINT
  if (argc <= 1) {
    usage(command);
    return 0;
  }

  int duration = std::stoi(argv[1], nullptr);  // NOLINT
  if (duration <= 0) {
    usage(command);
    return 1;
  }

  std::cout << "Run realtime loop for " << duration << " seconds" << std::endl;
  std::cout << "Check thread by 'ps -eTo pid,rtprio,pri,comm'" << std::endl;

  MyLoop my_loop;

  my_loop.configure();
  my_loop.start();

  std::this_thread::sleep_for(std::chrono::seconds(duration));

  my_loop.stop();

  std::cout << "Loop finished, running " << my_loop.getCount() << " times." << std::endl;
  return 0;
}
