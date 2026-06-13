add_library(lib_core_bvhfile STATIC
    channel.h
    channel.cpp
    bvhnode.h
    bvhnode.cpp
    bvhmotion.h
    bvhmotion.cpp
    bvhfile.h
    bvhfile.cpp
)
target_include_directories(lib_core_bvhfile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lib_core_bvhfile PUBLIC Qt6::Core Qt6::Gui)
