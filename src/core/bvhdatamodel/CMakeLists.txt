add_library(lib_core_bvhdatamodel STATIC
    bvh3djoint.h
    bvh3dbone.h
    pose_utils.h
    pose_utils.cpp
    bvhjointlistmodel.h
    bvhjointlistmodel.cpp
    bvhbonelistmodel.h
    bvhbonelistmodel.cpp
    bvh3dmodel.h
    bvh3dmodel.cpp
)

target_include_directories(lib_core_bvhdatamodel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(lib_core_bvhdatamodel PROPERTIES AUTOMOC ON)
target_link_libraries(lib_core_bvhdatamodel
    PUBLIC
        Qt6::Core
        Qt6::Gui
        lib_core_bvhfile
)
