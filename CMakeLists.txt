cmake_minimum_required(VERSION 3.16)

project(bvhviewer VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 6.5 COMPONENTS Quick Gui QuickControls2 REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

qt_add_executable(app_bvhviewer
    main.cpp
    src/core/channel.h
    src/core/channel.cpp
    src/core/bvhnode.h
    src/core/bvhnode.cpp
    src/core/bvhmotion.h
    src/core/bvhmotion.cpp
    src/core/bvhfile.h
    src/core/bvhfile.cpp
    src/ui/bvhscenemodel.h
    src/ui/bvhscenemodel.cpp
    src/ui/bvhscenelistmodel.h
    src/ui/bvhscenelistmodel.cpp
    src/ui/bvhviewitem.h
    src/ui/bvhviewitem.cpp
)

qt_add_qml_module(app_bvhviewer
    URI BvhViewer
    VERSION 1.0
    QML_FILES
        Main.qml
)

set_target_properties(app_bvhviewer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.bvhviewer
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "bvhviewer"
)

target_link_libraries(app_bvhviewer PRIVATE Qt6::Gui Qt6::Quick Qt6::QuickControls2)

# Enable testing
enable_testing()
add_subdirectory(tests/auto)

