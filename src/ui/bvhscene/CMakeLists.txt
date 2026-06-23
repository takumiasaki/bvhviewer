qt_add_library(lib_ui_bvhscene STATIC)

qt_add_qml_module(lib_ui_bvhscene
    URI BvhScene
    VERSION 1.0
    SOURCES
        scenemanager.h
        scenemanager.cpp
        bvhskeletonitem.h
        bvhskeletonitem.cpp
        skeletoncolorutils.h
        skeletoncolorutils.cpp
        camerasettings.h
        camerasettings.cpp
        viewportsettings.h
        viewportsettings.cpp
        bvhscene_foreign_types.h
)

target_include_directories(lib_ui_bvhscene
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/core/bvhdatamodel
)

target_link_libraries(lib_ui_bvhscene
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        lib_core_bvhdatamodel
        lib_core_bvhfile
)
