cmake_minimum_required(VERSION 3.16)
project(hw1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

add_executable(hw1
    src/hw1.cpp
    src/mesh_processor.cpp
    include/mesh_processor.h
)

# Add include directory for mesh_processor.h
target_include_directories(hw1 PRIVATE include)

target_link_libraries(hw1
    PRIVATE
        mesh_viewer
        geometry::halfedge  # geometry模块现在包含了MeshConverter
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
)

# Qt部署工具
if(WIN32)
    add_custom_command(TARGET hw1 POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --debug    # Debug版本
            --no-compiler-runtime
            --no-system-d3d-compiler
            \"$<TARGET_FILE:hw1>\"
        COMMENT "Deploying Qt libraries for hw1..."
    )
endif()