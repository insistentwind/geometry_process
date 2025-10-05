cmake_minimum_required(VERSION 3.16)
project(w2 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

add_executable(w2
    src/w2.cpp
)

target_link_libraries(w2
    PRIVATE
        mesh_viewer
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
)

# Qt部署设置
if(WIN32)
    add_custom_command(TARGET w2 POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --debug    # Debug构建
            --no-compiler-runtime
            --no-system-d3d-compiler
            \"$<TARGET_FILE:w2>\"
        COMMENT "Deploying Qt libraries for w2..."
    )
endif()