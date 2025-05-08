#include <iostream>
#include <thread>
#include "hid_foot_pedal/foot_pedal.h"

namespace {
std::sig_atomic_t exit_flag = 0;
void SignalHandler(const int signal) {exit_flag = signal;}
const UsbFootPedalInfo kQinHengPedal{
    0x1a86,
    0xe026,
    {
        {0x01, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    }
};
class CallbackHandler final : public FootPedalCallbacks {
  public:
    void OnFootpedalChange(const FootPedalState &value) override {
      switch (value) {
        case kPressed: std::cout << "Foot pedal pressed" << std::endl; break;
        case kReleased: std::cout << "Foot pedal released" << std::endl;  break;
      }
    }
};
}
int main() {
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  CallbackHandler cb;
  UsbFootPedal robot_pedal(exit_flag, kQinHengPedal, &cb);

  if (std::thread pedal_thread(&UsbFootPedal::Run, &robot_pedal); pedal_thread.joinable()) {
    pedal_thread.join();
  }

  return 0;
}