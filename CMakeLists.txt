# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.16)


project (mesh_works LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add Qt deployment settings
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

# 在添加geometry之前
add_subdirectory(thirdparty/eigen-3.4.0)


# 添加几何库（必须在其他项目之前）
add_subdirectory (geometry)

# 添加viewer库
add_subdirectory (viewer)

# 添加所有work项目
add_subdirectory (src/hw1)
add_subdirectory (src/hw2) 
add_subdirectory (src/hw3) 
add_subdirectory (src/hw4) 
add_subdirectory (src/hw5) 
add_subdirectory (src/hw6) 
add_subdirectory (src/hw7) 
# Qt部署设置（移动到每个work的CMakeLists.txt中）
if(WIN32)
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
endif()
