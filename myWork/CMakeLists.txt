cmake_minimum_required(VERSION 3.16)

project(myWork LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find OpenGL package
find_package(OpenGL REQUIRED)

add_executable(myWork
    myWork.cpp
    mainwindow.cpp
    glwidget.cpp
    objloader.cpp
)

target_include_directories(myWork PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(myWork PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    OpenGL::GL
)

# 使用 Qt 提供的部署工具
if(WIN32)
    # 获取 windeployqt 的路径
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    add_custom_command(TARGET myWork POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --debug    # 如果是 Debug 构建
            --no-compiler-runtime
            --no-system-d3d-compiler
            \"$<TARGET_FILE:myWork>\"
        COMMENT "Deploying Qt libraries using windeployqt..."
    )
endif()
