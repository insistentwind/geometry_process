cmake_minimum_required(VERSION 3.16)

# 创建几何库
add_library(geometry STATIC
    src/halfedge.cpp
    include/halfedge.h
)

# 设置包含目录
target_include_directories(geometry 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接Eigen库
find_package(Eigen3 REQUIRED)
target_link_libraries(geometry 
    PUBLIC 
        Eigen3::Eigen
)

# 设置编译属性
target_compile_features(geometry PUBLIC cxx_std_20)

# 设置别名，方便其他项目使用
add_library(geometry::halfedge ALIAS geometry)