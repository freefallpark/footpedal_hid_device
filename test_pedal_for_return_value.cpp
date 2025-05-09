#include <iomanip>
#include <iostream>
#include <optional>

#include "hid_foot_pedal/foot_pedal.h"

int main(const int argc, char *argv[]) {
  // 0: HANDLE INPUTS //////////////////////////////////////////////////////////////////////////////
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <vendor_id> <product_id>" << std::endl;
  }
  // parse argv[1] and argv[2] as hex
  char *endptr = nullptr;
  const unsigned long vid = std::strtoul(argv[1], &endptr, 16);
  if (*endptr != '\0' || vid > 0xFFFF) {
    std::cerr << "Invalid vendor ID: " << argv[1] << "\n";
    return 1;
  }
  const unsigned long pid = std::strtoul(argv[2], &endptr, 16);
  if (*endptr != '\0' || pid > 0xFFFF) {
    std::cerr << "Invalid product ID: " << argv[2] << "\n";
    return 1;
  }
  const auto vendor_id = static_cast<unsigned short>(vid);
  const auto product_id = static_cast<unsigned short>(pid);

  // 1: INITIALIZE HID LIBRARY /////////////////////////////////////////////////////////////////////
  if (hid_init()) {
    std::cerr << "hid_init failed" << std::endl;
    return 1;
  }

  // 1: OPEN PEDAL /////////////////////////////////////////////////////////////////////////////////
  const auto pedal_device = hid_open(vendor_id, product_id, nullptr);
  if (pedal_device == nullptr) {
    std::cerr << "Failed to open footpedal:\n" << std::endl;
    std::cerr << "\tVendor ID: " << std::hex << vendor_id << std::endl;
    std::cerr << "\tProduct ID: " << std::hex << product_id << std::endl;
    hid_exit();
    return 0;
  }

  // Determine Foot pedal 'pressed' behavior
  constexpr size_t MAX_SIZE = 64;
  std::vector<unsigned char> down_buffer(MAX_SIZE);
  std::cout << "Press and Hold foot pedal" << std::endl;
  if (const int down_len = hid_read(
      pedal_device,
      down_buffer.data(),
      down_buffer.size()); down_len > 0) {
    down_buffer.resize(down_len);
    std::cout << "Foot Pedal down produces:" << std::endl;
    for (const auto c : down_buffer) {
      std::cout << ' '
          << std::hex << std::setw(2) << std::setfill('0')
          << static_cast<int>(c);
    }
    std::cout << std::dec << '\n';

    // Determine Foot pedal 'released' behavior
    std::cout << "Release foot pedal" << std::endl;
    std::vector<unsigned char> up_buffer(MAX_SIZE);
    if (const int up_len = hid_read(
        pedal_device,
        up_buffer.data(),
        up_buffer.size()); up_len > 0) {
      up_buffer.resize(up_len);
      std::cout << "Foot Pedal up produces:" << std::endl;
      for (const auto c : up_buffer) {
        std::cout << ' '
            << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(c);
      }
      std::cout << std::dec << '\n';
    }
  }

  hid_close(pedal_device);
  hid_exit();
  return 0;
}
