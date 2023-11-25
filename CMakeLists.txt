cmake_minimum_required(VERSION 3.18)
project(StpLynxHardwareServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(StpLynxHardwareServer main.cpp
        headers/gyro.h
        httplib.h)
target_link_libraries(StpLynxHardwareServer dl)