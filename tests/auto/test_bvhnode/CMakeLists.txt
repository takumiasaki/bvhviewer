cmake_minimum_required(VERSION 3.16)

project(test_bvhnode LANGUAGES CXX)

find_package(Qt6 6.5 COMPONENTS Test Core Gui REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test_bvhnode
    tst_bvhnode.cpp
    ../../../src/core/channel.h
    ../../../src/core/channel.cpp
    ../../../src/core/bvhnode.h
    ../../../src/core/bvhnode.cpp
)

target_link_libraries(test_bvhnode PRIVATE Qt6::Test Qt6::Core Qt6::Gui)
target_include_directories(test_bvhnode PRIVATE ../../../src/core)

add_test(NAME test_bvhnode COMMAND test_bvhnode)
