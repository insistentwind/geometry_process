# 项目结构

## 项目概述

这个项目使用半边数据结构来处理三维网格，支持多个作业项目共享同一个几何库。

## 项目结构

```
mesh_works/
├── CMakeLists.txt           # 主CMake配置文件
├── GEOMETRY_README.md      # 项目说明文档
├── geometry/               # 共享几何库
│   ├── CMakeLists.txt      # 几何库CMake配置
│   ├── include/
│   │   └── halfedge.h      # 半边数据结构头文件
│   └── src/
│       └── halfedge.cpp    # 半边数据结构实现
├── viewer/                 # 共享的3D查看器库
│   ├── CMakeLists.txt
│   ├── include/
│   └── src/
├── src/                    # 各个作业项目
│   ├── hw1/               # 作业1
│   │   ├── CMakeLists.txt
│   │   └── src/
│   │       └── hw1.cpp
│   ├── hw2/               # 作业2 (示例)
│   │   ├── CMakeLists.txt
│   │   └── src/
│   │       └── hw2.cpp
│   └── ...                # 更多作业项目
└── thirdparty/            # 第三方库（如Eigen）
```

## 几何库特性

### 半边数据结构 (`geometry::HalfEdgeMesh`)

- **命名空间**: `geometry`
- **头文件**: `#include <halfedge.h>`
- **特性**:
  - 高效的网格拓扑表示
  - 支持复杂的网格操作
  - 完整的数据结构验证
  - 详细的文档和错误处理

### 主要类

1. **`geometry::Vertex`** - 顶点类
2. **`geometry::Face`** - 面类
3. **`geometry::HalfEdge`** - 半边类
4. **`geometry::HalfEdgeMesh`** - 半边网格类

## 如何创建新的作业项目

参考 `src/hw2/` 目录的实现，只需要：

1. 创建新的项目目录
2. 添加CMakeLists.txt（链接 `geometry::halfedge`）
3. 在主程序中 `#include <halfedge.h>`
4. 使用 `geometry::HalfEdgeMesh` 进行网格处理

## 编译和运行

```bash
cmake -B build -G Ninja
cmake --build build
./build/bin/hw1.exe
./build/bin/hw2.exe
```