cmake_minimum_required(VERSION 3.16)

project(test_bvh3dmodel LANGUAGES CXX)

find_package(Qt6 6.5 COMPONENTS Test Core Gui REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test_bvh3dmodel
    tst_bvh3dmodel.cpp
)

target_compile_definitions(test_bvh3dmodel PRIVATE TEST_BVH_DIR="${CMAKE_SOURCE_DIR}/bvhfiles")

target_link_libraries(test_bvh3dmodel PRIVATE Qt6::Test Qt6::Core Qt6::Gui)
target_link_libraries(test_bvh3dmodel PRIVATE lib_core_bvhdatamodel lib_core_bvhfile)

add_test(NAME test_bvh3dmodel COMMAND test_bvh3dmodel)
