cmake_minimum_required(VERSION 3.31)
project(footpedal_hid_device)

set(CMAKE_CXX_STANDARD 17)

find_package(hidapi REQUIRED)
add_library(FootPedal hid_foot_pedal/foot_pedal.cpp)
target_link_libraries(FootPedal PRIVATE hidapi::libusb)

add_executable(footpedal_hid_device main.cpp
        hid_foot_pedal/foot_pedal.cpp
        hid_foot_pedal/foot_pedal.h)
target_link_libraries(footpedal_hid_device PRIVATE FootPedal)
add_executable(test_pedal_for_return_value test_pedal_for_return_value.cpp)
target_link_libraries(test_pedal_for_return_value PRIVATE FootPedal)
