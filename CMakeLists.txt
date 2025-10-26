# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.16)


project (mesh_works LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC统一使用 UTF-8 避免源文件编码警告 (C4828)
if(MSVC)
    add_compile_options(/utf-8)
    # 可选：禁用 C4828 警告（不推荐，应该修复源文件编码）
    # add_compile_options(/wd4828)
endif()

#-------------- Windows SDK选择--------------
# 可选：强制指定一个较新的 Windows11 SDK版本（示例：10.0.22621.0）
# 如需关闭强制，可在配置时加 -DFORCE_WIN11_SDK=OFF
option(FORCE_WIN11_SDK "Force Windows10 SDK (19041) selection" ON)
if(WIN32 AND FORCE_WIN10_SDK)
 # 如果未安装该版本会导致配置失败，请确保在 VS Installer 中已安装对应 Windows SDK
 set(CMAKE_SYSTEM_VERSION 10.0.19041.0)
 # 针对 VS生成器的额外变量（Ninja 下通常只看 CMAKE_SYSTEM_VERSION）
 set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION 10.0.19041.0)
endif()
# ----------------------------------------------

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
