#include "../ratatoskr/concurrent.hpp"
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

int main() {
  using namespace rat::concurrent;
  using namespace std::chrono_literals;

  auto log = [](auto tag, auto x) {
    static std::mutex io_mutex;
    std::lock_guard lock{io_mutex};
    std::cout << tag << ": " << x << " @thread #" << std::this_thread::get_id()
              << std::endl;
  };

  auto [sn, rc] = make_channel<int>();

  auto produce = [&log](auto sn) {
    for (int i = 0; i < 10; ++i) {
      log("send   ", i);
      sn.push(i);
      std::this_thread::sleep_for(1s);
    }
    log("send   ", "close");
    sn.close();
  };

  auto consume = [&log](auto rc) {
    try {
      while (true) {
        log("receive", rc.next());
      }
    }
    catch (const close_channel &) {
      log("receive", "close");
    }
  };

  std::thread producer{produce, std::move(sn)};
  std::thread consumer{consume, std::move(rc)};
  producer.join();
  consumer.join();
}
