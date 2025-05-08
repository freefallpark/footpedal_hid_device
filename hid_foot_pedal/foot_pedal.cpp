// Copyright (c) 2025, Ultradent Products Inc. All rights reserved.
#include "foot_pedal.h"

#include <iomanip>
#include <iostream>
#include <thread>

UsbFootPedal::UsbFootPedal(const volatile sig_atomic_t &exit_flag,
                           UsbFootPedalInfo pedal_info,
                           FootPedalCallbacks *callbacks): exit_flag_(exit_flag),
                                                           pedal_info_(std::move(pedal_info)),
                                                           pedal_device_(nullptr),
                                                           callbacks_(callbacks) {}

int UsbFootPedal::Run() {
  if (!Init()) return 1;
  std::cout << "RobotFootPedal initialized" << std::endl;
  std::vector<unsigned char> buf(64);

  while (exit_flag_ == 0) {
    std::unique_lock lock(mtx_);
    if (const int len = hid_read(pedal_device_, buf.data(), buf.size()); len > 0) {
      lock.unlock();
      buf.resize(len);
      if (buf == pedal_info_.expected_behavior.pressed_value) {
        if (callbacks_) {callbacks_->OnFootpedalChange(kPressed);}
        SetPedal(kPressed);
      }
      else if (buf == pedal_info_.expected_behavior.released_value) {
        if (callbacks_) {callbacks_->OnFootpedalChange(kReleased);}
        SetPedal(kReleased);
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
  }

  std::cout << "\nRobotFootPedal closing" << std::endl;
  hid_close(pedal_device_);
  hid_exit();
  return 0;
}

FootPedalState UsbFootPedal::GetState() {
  std::lock_guard lock(mtx_);
  return pedal_state_;
}

std::optional<FootPedalBehavior> UsbFootPedal::DetermineExpectedPedalValue(
    const unsigned short vendor_id,
    const unsigned short product_id) {
  if (const std::optional<hid_device *> opt_pedal = Init(vendor_id, product_id)) {
    // Determine Foot pedal 'pressed' behavior
    constexpr size_t MAX_SIZE = 64;
    std::vector<unsigned char> down_buffer(MAX_SIZE);
    std::cout << "Press and Hold foot pedal" << std::endl;
    if (const int down_len =
        hid_read(opt_pedal.value(), down_buffer.data(), down_buffer.size()); down_len > 0) {
      down_buffer.resize(down_len);
      std::cout << "Foot Pedal down produces:" << std::endl;
      for (const auto c:down_buffer) {
        std::cout << ' '
            << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(c);
      }
      std::cout << std::dec << '\n';

      // Determine Foot pedal 'released' behavior
      std::cout << "Release foot pedal" << std::endl;
      std::vector<unsigned char> up_buffer(MAX_SIZE);
      if (const int up_len =
          hid_read(opt_pedal.value(), up_buffer.data(), up_buffer.size()); up_len > 0) {
        up_buffer.resize(up_len);
        std::cout << "Foot Pedal up produces:" << std::endl;
        for (const auto c : up_buffer) {
          std::cout << ' '
              << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(c);
        }
        std::cout << std::dec << '\n';
        hid_close(opt_pedal.value());
        hid_exit();
        return FootPedalBehavior{down_buffer,up_buffer};
      }
    }

    hid_close(opt_pedal.value());
  }
  hid_exit();
  return std::nullopt;
}

bool UsbFootPedal::Init() {
  std::lock_guard lock(mtx_);

  // 0: Init HID Library
  if (hid_init()) {
    std::cerr << "hid_init failed" << std::endl;
    return false;
  }
  // 1: Open Pedal using Vendor and Product ID's
  pedal_device_ = hid_open(pedal_info_.vendor_id, pedal_info_.product_id, nullptr);
  if (pedal_device_ == nullptr) {
    std::cerr << "Failed to open footpedal:\n"<< std::endl;
    std::cerr << "\tVendor ID: " << std::hex << pedal_info_.vendor_id << std::endl;
    std::cerr << "\tProduct ID: " << std::hex << pedal_info_.product_id << std::endl;
    hid_exit();
    return false;
  }
  if (hid_set_nonblocking(pedal_device_, 1) != 0) {
    std::cerr << "Failed to set nonblocking on pedal:\n";
    return false;
  }
  return true;
}

hid_device * UsbFootPedal::Init(const unsigned short vendor_id, const unsigned short product_id) {
  // 0: Init HID Library
  if (hid_init()) {
    std::cerr << "hid_init failed" << std::endl;
    return nullptr;
  }

  // 1: Open Pedal using Vendor and Product ID's
  const auto pedal_device = hid_open(vendor_id, product_id, nullptr);
  if (pedal_device == nullptr) {
    std::cerr << "Failed to open footpedal:\n"<< std::endl;
    std::cerr << "\tVendor ID: " << std::hex << vendor_id << std::endl;
    std::cerr << "\tProduct ID: " << std::hex << product_id << std::endl;
    hid_exit();
    return nullptr;
  }

  return pedal_device;
}

void UsbFootPedal::SetPedal(const FootPedalState state) {
  std::lock_guard lock(mtx_);
  pedal_state_ = state;
}
