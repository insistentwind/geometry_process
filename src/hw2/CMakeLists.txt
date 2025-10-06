cmake_minimum_required(VERSION 3.16)
project(hw2 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

add_executable(hw2
    src/hw2.cpp
    include/mesh_processor.h
    src/mesh_processor.cpp)

target_include_directories(hw2 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(hw2
    PRIVATE
        mesh_viewer
        geometry::halfedge  # 使用共享的几何库
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
)

# Qt部署设置
if(WIN32)
    add_custom_command(TARGET hw2 POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --debug
            --no-compiler-runtime
            --no-system-d3d-compiler
            \"$<TARGET_FILE:hw2>\"
        COMMENT "Deploying Qt libraries for hw2..."
    )
endif()