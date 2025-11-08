cmake_minimum_required(VERSION 3.16)
project(hw8 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

add_executable(hw8
    src/hw8.cpp
    include/mesh_processor.h
    src/mesh_processor.cpp)

target_include_directories(hw8 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(hw8
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
    add_custom_command(TARGET hw8 POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --debug
            --no-compiler-runtime
            --no-system-d3d-compiler
            \"$<TARGET_FILE:hw8>\"
        COMMENT "Deploying Qt libraries for hw8..."
    )
endif()