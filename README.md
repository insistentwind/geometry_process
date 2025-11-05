# Mesh Works - 3D 网格处理项目

## 项目概述

这是一个基于**半边数据结构 (Half-Edge)** 的 3D 网格处理框架，使用 **Qt6 + OpenGL + Eigen** 构建，支持多个独立的几何处理作业项目。

---

## 项目架构

### 核心模块

#### 1. geometry - 几何库 (静态库)
- **半边数据结构** (`HalfEdgeMesh`)
- **网格转换器** (`MeshConverter`)
- 依赖：Eigen 3.4.0, Qt6::Core, Qt6::Gui

#### 2. viewer - 3D 查看器库 (静态库)
- **OpenGL 渲染窗口** (`GLWidget`)
- **主窗口界面** (`MainWindow`)
- **OBJ 文件加载器** (`ObjLoader`)
- **异步网格处理器** (`AsyncMeshProcessor`)
- 依赖：Qt6::Widgets, Qt6::OpenGL, Qt6::OpenGLWidgets, OpenGL

#### 3. hw1 - hw7 - 作业项目 (可执行文件)
- 每个作业实现自己的 `MeshProcessor::processGeometry()`
- 链接：geometry, viewer 库

---

## 目录结构

```
mesh_works/
├── CMakeLists.txt    # 顶层 CMake 配置
├── CMakePresets.json    # Visual Studio CMake 预设
├── .gitignore
├── README.md  # 本文档
├── GEOMETRY_README.md       # 几何库旧文档
│
├── geometry/       # 几何库
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── halfedge.h         # 半边数据结构定义
│   │   └── mesh_converter.h   # 网格转换工具
│   └── src/
│       ├── halfedge.cpp
│       └── mesh_converter.cpp
│
├── viewer/    # 3D 查看器库
│   ├── CMakeLists.txt
│ ├── include/
│   │   ├── glwidget.h         # OpenGL 渲染窗口
│   │   ├── mainwindow.h# 主窗口
│   │   ├── objloader.h        # OBJ 加载器
│   │   └── async_mesh_processor.h
│   ├── src/
│   │   ├── glwidget.cpp
│   │   ├── mainwindow.cpp
│   │   ├── objloader.cpp
│   │└── async_mesh_processor.cpp
│   └── shaders/
│       ├── basic.vert       # 顶点着色器
│       ├── basic.frag         # 片段着色器
│       └── resources.qrc   # Qt 资源文件
│
├── src/                # 作业项目
│   ├── hw1/    # 作业 1
│   │   ├── CMakeLists.txt
│   │   ├── include/mesh_processor.h
│   │   └── src/
│   │       ├── hw1.cpp        # 主程序
│   │       └── mesh_processor.cpp
│   ├── hw2/     # 作业 2: 曲率计算
│   ├── hw3/           # 作业 3
│   ├── hw4/     # 作业 4
│   ├── hw5/            # 作业 5
│   ├── hw6/# 作业 6: ARAP 变形
│   └── hw7/ # 作业 7
│
├── thirdparty/   # 第三方库
│   └── eigen-3.4.0/     # Eigen 线性代数库
│
├── out/    # 构建输出 (Visual Studio)
│   └── build/
│       └── x64-debug/
│       ├── bin/           # 可执行文件 (hw1.exe ~ hw7.exe)
│           └── lib/           # 静态库
│
├── build/            # 命令行构建目录 (可删除)
└── build_vs/       # 旧的 VS 构建目录 (可删除)
```

---

## 技术栈

| 组件 | 版本 | 用途 |
|------|------|------|
| **C++ 标准** | C++20 | 语言标准 |
| **CMake** | 3.16+ | 构建系统 |
| **Qt6** | 6.9.3 | GUI 框架 + OpenGL 支持 |
| **Eigen** | 3.4.0 | 线性代数库 |
| **OpenGL** | - | 3D 渲染 |
| **Ninja** | - | CMake 生成器 |
| **MSVC** | 19.50+ | 编译器 (Visual Studio 2026) |

---

## 编译与运行

### 方式 1: Visual Studio 2022/2026 (推荐)

1. **打开项目**
   ```
   File → Open → CMake → 选择 CMakeLists.txt
   ```

2. **配置预设**
   - 选择 `x64-debug` 或 `x64-release`

3. **构建**
   ```
   生成 → 全部生成
   ```

4. **运行**
   - 选择启动项 (hw1 ~ hw7)
   - 按 F5 运行

### 方式 2: 命令行 (Ninja)

```bash
# 配置
cmake --preset x64-debug

# 编译
cmake --build out/build/x64-debug

# 运行
out/build/x64-debug/bin/hw1.exe
```

---

## Qt 部署说明

### 重要修复 (2024-12)

**问题**: 多个项目并行构建时，windeployqt 冲突导致 `Cannot create generic` 错误。

**解决方案**: 
- 只在 hw1 执行 windeployqt（所有项目共享 `out/build/x64-debug/bin`）
- hw2-hw7 不再执行 windeployqt

```cmake
# hw1/CMakeLists.txt - 保留部署命令
add_custom_command(TARGET hw1 POST_BUILD
    COMMAND windeployqt ...
)

# hw2-hw7/CMakeLists.txt - 移除部署命令
# Qt deployment is handled by hw1 target to avoid conflicts
```

---

## 如何创建新作业

### 步骤 1: 复制模板
```bash
cp -r src/hw1 src/hw8
```

### 步骤 2: 修改 CMakeLists.txt
```cmake
# src/hw8/CMakeLists.txt
project(hw8 LANGUAGES CXX)

add_executable(hw8
    src/hw8.cpp
    src/mesh_processor.cpp
    include/mesh_processor.h
)

target_link_libraries(hw8
    PRIVATE
        mesh_viewer
        geometry::halfedge
        Qt6::Core Qt6::Gui Qt6::Widgets
        Qt6::OpenGL Qt6::OpenGLWidgets
)

# 不需要 windeployqt (由 hw1 统一处理)
```

### 步骤 3: 实现算法
```cpp
// src/hw8/src/mesh_processor.cpp
void MeshProcessor::processGeometry() {
    // 在这里实现你的网格处理算法
    for (auto& vertex : mesh.vertices) {
        // 处理顶点...
    }
}
```

### 步骤 4: 添加到主 CMakeLists.txt
```cmake
# 顶层 CMakeLists.txt
add_subdirectory(src/hw8)
```

---

## 核心 API 使用

### geometry::HalfEdgeMesh

```cpp
#include <halfedge.h>

// 遍历所有顶点
for (auto& vertex : mesh.vertices) {
    Eigen::Vector3d pos = vertex->position;
    Eigen::Vector3d color = vertex->color;
}

// 遍历一阶邻域
geometry::HalfEdge* hf = vertex->halfEdge;
do {
    geometry::Vertex* neighbor = hf->getEndVertex();
hf = hf->pair->next;
} while (hf != vertex->halfEdge);

// 判断边界点
if (vertex->isBoundary()) {
    // 边界处理...
}
```

### MeshConverter

```cpp
#include <mesh_converter.h>

// Qt 数据 → HalfEdgeMesh
geometry::MeshConverter::buildMeshFromQtData(mesh, vertices, indices);

// HalfEdgeMesh → Qt 数据
auto [new_vertices, new_indices] = 
    geometry::MeshConverter::convertMeshToQtData(mesh);
```

---

## 清理构建文件

### 可以安全删除的目录：
```bash
# 命令行构建目录
rm -rf build build_vs

# Visual Studio 缓存 (会自动重建)
rm -rf out .vs
```

### 应该保留的目录：
- `geometry/` - 几何库源码
- `viewer/` - 查看器源码
- `src/` - 作业源码
- `thirdparty/` - 第三方库

---

## 作业项目说明

| 作业 | 功能 | 关键算法 |
|------|------|----------|
| hw1 | 基础网格操作 | - |
| hw2 | **曲率计算** | Mean Curvature, Cotangent Curvature, Gaussian Curvature |
| hw3 | - | - |
| hw4 | - | - |
| hw5 | - | - |
| hw6 | **ARAP 变形** | Local-Global 迭代优化, SVD 旋转提取 |
| hw7 | - | - |

---

## 常见问题

### 1. `Cannot create generic` 错误
**原因**: 多个项目同时运行 windeployqt  
**解决**: 已修复，只在 hw1 执行部署

### 2. 链接错误 (LNK2019)
**原因**: x86/x64 库混用  
**解决**: 删除 `out` 目录，在 VS 中"删除缓存并重新配置"

### 3. UTF-8 编码警告
**原因**: MSVC 默认使用 GBK  
**解决**: 已在顶层 CMakeLists.txt 添加 `/utf-8`

### 4. Qt 找不到 DLL
**原因**: windeployqt 未执行  
**解决**: 重新生成 hw1 项目

---

## 许可证

本项目仅供学习使用。

---

## 开发者信息

- GitHub: https://github.com/insistentwind/geometry_process
- Qt 版本: 6.9.3
- Visual Studio: 2026 Preview

---

**最后更新**: 2025-10
**重要修复**: windeployqt 并行冲突问题