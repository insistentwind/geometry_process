cmake_minimum_required(VERSION 3.16)

# 几何库项目
add_library(geometry STATIC
    src/halfedge.cpp
    src/mesh_converter.cpp
    include/halfedge.h
    include/mesh_converter.h
)

# 设置包含目录
target_include_directories(geometry 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 查找Eigen库
find_package(Eigen3 REQUIRED)

# 查找Qt6库（用于MeshConverter）
find_package(Qt6 REQUIRED COMPONENTS Core Gui)

target_link_libraries(geometry 
    PUBLIC 
        Eigen3::Eigen
        Qt6::Core
        Qt6::Gui
)

# 设置编译标准
target_compile_features(geometry PUBLIC cxx_std_20)

# 创建别名以便其他项目使用
add_library(geometry::halfedge ALIAS geometry)