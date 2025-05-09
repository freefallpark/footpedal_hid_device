// Copyright (c) 2025, Ultradent Products Inc. All rights reserved.
#include "foot_pedal.h"

#include <chrono>
#include <optional>
#include <iostream>
#include <thread>
#include <oneapi/tbb/detail/_template_helpers.h>

UsbFootPedal::UsbFootPedal(const volatile sig_atomic_t &exit_flag,
                           UsbFootPedalInfo pedal_info,
                           FootPedalCallbacks *callbacks)
    : exit_flag_(exit_flag),
      pedal_info_(std::move(pedal_info)),
      callbacks_(callbacks) {}
int UsbFootPedal::Run() {
  if (!Init()) return 1;
  std::cout << "RobotFootPedal initialized" << std::endl;
  std::vector<unsigned char> buf(64);

  while (exit_flag_ == 0) {
    hid_device_ *device = nullptr; {
      std::lock_guard lock(mtx_);
      if (pedal_device_) {
        device = pedal_device_->get();
      }
    }
    if (const int len = hid_read(device, buf.data(), buf.size()); len > 0) {
      buf.resize(len);

      // Compute next state
      FootPedalState next = (buf == pedal_info_.expected_behavior.pressed_value) ? kPressed :
                            (buf == pedal_info_.expected_behavior.released_value) ? kReleased :
                                                                                   pedal_state_;
      if (next != pedal_state_) {
        if (callbacks_) {
          callbacks_->OnFootpedalChange(next);
        }
        std::lock_guard lock(mtx_);
        pedal_state_ = next;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
  }

  std::cout << "\nRobotFootPedal closing" << std::endl;
  return 0;
}
FootPedalState UsbFootPedal::GetState() {
  std::lock_guard lock(mtx_);
  return pedal_state_;
}
bool UsbFootPedal::Init() {
  std::lock_guard lock(mtx_);
  // 0: Initialize device
  hid_device* raw = hid_open(pedal_info_.vendor_id, pedal_info_.product_id, nullptr);
  if (raw == nullptr) {
    std::cerr << "Failed to open footpedal:\n"<< std::endl;
    std::cerr << "\tVendor ID: " << std::hex << pedal_info_.vendor_id << std::endl;
    std::cerr << "\tProduct ID: " << std::hex << pedal_info_.product_id << std::endl;
    return false;
  }
  pedal_device_.emplace(raw);
  // 1: Open Pedal using Vendor and Product ID's
  if (hid_set_nonblocking(pedal_device_->get(), 1) != 0) {
    std::cerr << "Failed to set nonblocking on pedal:\n";
    return false;
  }
  return true;
}
void UsbFootPedal::SetPedal(const FootPedalState state) {
  std::lock_guard lock(mtx_);
  pedal_state_ = state;
}
