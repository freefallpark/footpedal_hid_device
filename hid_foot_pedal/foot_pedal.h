// Copyright (c) 2025, Ultradent Products Inc. All rights reserved.

#ifndef FOOT_PEDAL_H
#define FOOT_PEDAL_H

#include <hidapi/hidapi.h>
#include <csignal>
#include <mutex>
#include <optional>
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
  unsigned short vendor_id;
  unsigned short product_id;
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

    /**
     * @brief Determines Behavior of a usb foot pedal
     * @param vendor_id Vendor ID of usb device
     * @param product_id Product ID of usb device
     * @return Upon Failure returns std::nullopt, otherwise returns the press and release behavior
     * of the pedal
     */
    static std::optional<FootPedalBehavior> DetermineExpectedPedalValue(
        unsigned short vendor_id,
        unsigned short product_id);

  private:
    /**
     * @brief Initializes the usb foot pedal
     * @return true upon success, false upon failure
     */
    bool Init();

    /**
     *
     * @param vendor_id Vendor ID of usb device
     * @param product_id Product ID of usb device
     * @return returns pointer to hid_device upon success, nullptr upon failure
     */
    static hid_device *Init(unsigned short vendor_id,
                            unsigned short product_id);

    /**
     * @brief Setter for pedal state
     * @param state State to set.
     */
    void SetPedal(FootPedalState state);

    std::mutex mtx_;
    const volatile sig_atomic_t &exit_flag_;
    const UsbFootPedalInfo pedal_info_;
    hid_device *pedal_device_;
    FootPedalCallbacks *callbacks_;
    FootPedalState pedal_state_ = FootPedalState::kReleased;
};



#endif //FOOT_PEDAL_H
