cmake_minimum_required(VERSION 3.16)
project(mesh_viewer LANGUAGES CXX)

# Qt自动处理设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
find_package(OpenGL REQUIRED)

# 明确列出所有源文件和头文件
set(VIEWER_HEADERS
    include/glwidget.h
    include/mainwindow.h
    include/objloader.h
    include/async_mesh_processor.h
)

set(VIEWER_SOURCES
    src/glwidget.cpp
    src/mainwindow.cpp
    src/objloader.cpp
    src/async_mesh_processor.cpp
)

# Create viewer library
add_library(mesh_viewer STATIC
    ${VIEWER_HEADERS}
    ${VIEWER_SOURCES}
)

target_include_directories(mesh_viewer 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(mesh_viewer
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        OpenGL::GL
)