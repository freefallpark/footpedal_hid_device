// Copyright (c) 2025, Ultradent Products Inc. All rights reserved.

#ifndef FOOT_PEDAL_H
#define FOOT_PEDAL_H

#include <hidapi/hidapi.h>
#include <csignal>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <vector>


/**
 * @struct FootPedalBehavior
 * @brief Defines the expected value upon 'pressed' event and 'released' event
 */
struct FootPedalBehavior {
  /**
   * @brief Value when pressed
   */
  std::vector<unsigned char> pressed_value;
  /**
   * @brief Value when released.
   */
  std::vector<unsigned char> released_value;
};

/**
 * @struct UsbFootPedalInfo
 * @brief Information that defines a specific usb footpedal
 */
struct UsbFootPedalInfo {
  unsigned short vendor_id{};
  unsigned short product_id{};
  FootPedalBehavior expected_behavior;
};

/**
 * @enum FootPedalState
 * @brief Possible states of a foot pedal.
 */
enum FootPedalState : int16_t {
  kReleased, kPressed
};

/**
 * @class FootPedalCallbacks
 * @brief Events triggered by teh FootPedal class.
 */
class FootPedalCallbacks {
  public:
    FootPedalCallbacks() = default;
    FootPedalCallbacks(const FootPedalCallbacks&) = delete;
    FootPedalCallbacks& operator=(const FootPedalCallbacks&) = delete;
    virtual ~FootPedalCallbacks() = default;

    virtual void OnFootpedalChange(const FootPedalState& value) = 0;
};

/**
 * @class UsbFootPedal
 * @brief Provides an API to access whether a usb Footpedal is in the 'pressed' or
 * 'released' state
 */
class UsbFootPedal {
  public:
    /**
     * @brief
     * @param exit_flag External Exit flag (i.e. ctl + c exit)
     * @param pedal_info UsbPedalInfo object describing the hardware you would like to use.
     * @param callbacks  Optional callbacks. If desired pass a pointer to a FootPedalCallback
     * object. These events will get triggered when appropriate.
     */
    explicit UsbFootPedal(const volatile sig_atomic_t &exit_flag,
                          UsbFootPedalInfo pedal_info,
                          FootPedalCallbacks *callbacks);

    /**
     * @brief Blocking run method for the foot pedal. Continuously checks
     * @return return 0 up on safe exit, 1 upon error state
     */
    int Run();

    /**
     * @brief Getter for current foot pedal state.
     * @return State of the foot pedal.
     */
    FootPedalState GetState();

  private:
    struct HidLibrary {
      HidLibrary() {
        if (hid_init()) throw std::runtime_error("hid_init failed");
      }
      ~HidLibrary() noexcept { hid_exit(); }
      HidLibrary(const HidLibrary&) = delete;
      HidLibrary& operator=(const HidLibrary&) = delete;
    };
    struct HidDeviceHandle {
      HidDeviceHandle() = delete;
      explicit HidDeviceHandle(hid_device *device) : device_(device) {
        if (!device_) throw std::runtime_error("Failed to open device");
      }
      ~HidDeviceHandle() noexcept { if (device_) hid_close(device_); }
      HidDeviceHandle(const HidDeviceHandle&) = delete;
      HidDeviceHandle& operator=(const HidDeviceHandle&) = delete;
      HidDeviceHandle(HidDeviceHandle&& o)  noexcept : device_(o.device_) {o.device_ = nullptr;}
      HidDeviceHandle& operator=(HidDeviceHandle&& o) noexcept {
        if (device_) hid_close(device_);
        device_ = o.device_;
        o.device_ = nullptr;
        return *this;
      }
      [[nodiscard]] hid_device * get() const noexcept { return device_; }
      private:
        hid_device *device_;
    };

    /**
     * @brief Initializes the usb foot pedal
     * @return true upon success, false upon failure
     */
    bool Init();

    /**
     * @brief Setter for pedal state
     * @param state State to set.
     */
    void SetPedal(FootPedalState state);

    std::mutex mtx_;
    const volatile sig_atomic_t &exit_flag_;
    const UsbFootPedalInfo pedal_info_;
    HidLibrary hid_library_;
    std::optional<HidDeviceHandle> pedal_device_;
    FootPedalCallbacks *callbacks_;
    FootPedalState pedal_state_ = FootPedalState::kReleased;
};



#endif //FOOT_PEDAL_H
