cmake_minimum_required(VERSION 3.16)

project(bvhviewer VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 6.5 COMPONENTS Quick Gui QuickControls2 Quick3D REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

qt_add_executable(app_bvhviewer
    main.cpp
)

add_subdirectory(src/core/bvhfile)
add_subdirectory(src/core/bvhdatamodel)
add_subdirectory(src/ui/bvhscene)

qt_add_qml_module(app_bvhviewer
    URI BvhViewer
    VERSION 1.0
    QML_FILES
        Main.qml
        SideBar.qml
        Bvh3DView.qml
)

set_target_properties(app_bvhviewer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.bvhviewer
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "bvhviewer"
)

target_link_libraries(app_bvhviewer PRIVATE Qt6::Gui Qt6::Quick Qt6::QuickControls2 Qt6::Quick3D)
target_link_libraries(app_bvhviewer PRIVATE lib_core_bvhfile lib_core_bvhdatamodel lib_ui_bvhscene)

# Enable testing
enable_testing()
add_subdirectory(tests/auto)
