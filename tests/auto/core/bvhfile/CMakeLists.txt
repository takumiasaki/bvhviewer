cmake_minimum_required(VERSION 3.16)

project(bvhviewer_tests)

enable_testing()

add_subdirectory(test_channel)
add_subdirectory(test_bvhnode)
add_subdirectory(test_bvhmotion)
add_subdirectory(test_bvhfile)
