cmake_minimum_required(VERSION 3.16)

project(test_channel LANGUAGES CXX)

find_package(Qt6 6.5 COMPONENTS Test Core REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test_channel
    tst_channel.cpp
    ../../../src/core/channel.h
    ../../../src/core/channel.cpp
)

target_link_libraries(test_channel PRIVATE Qt6::Test Qt6::Core)
target_include_directories(test_channel PRIVATE ../../../src/core)

add_test(NAME test_channel COMMAND test_channel)
