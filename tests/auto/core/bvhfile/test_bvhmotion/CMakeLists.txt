cmake_minimum_required(VERSION 3.16)

project(test_bvhmotion LANGUAGES CXX)

find_package(Qt6 6.5 COMPONENTS Test Core REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test_bvhmotion
    tst_bvhmotion.cpp
)

target_link_libraries(test_bvhmotion PRIVATE Qt6::Test Qt6::Core)
target_link_libraries(test_bvhmotion PRIVATE lib_core_bvhfile)

add_test(NAME test_bvhmotion COMMAND test_bvhmotion)
