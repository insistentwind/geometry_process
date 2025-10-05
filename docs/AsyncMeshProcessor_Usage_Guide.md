# 异步网格处理器使用指南

## 概述

`AsyncMeshProcessor` 是一个封装在 `viewer` 模块中的通用异步处理框架，用于在独立线程中执行耗时的网格处理操作，避免Qt界面无响应。

## 主要特性

- ? **非阻塞UI**: 耗时操作在独立线程中执行，主界面保持响应
- ? **通用接口**: 使用函数对象，可以轻松集成任何处理逻辑
- ? **信号通知**: 提供处理开始、完成、错误和进度的信号
- ? **线程安全**: 自动管理工作线程的生命周期
- ? **易于复用**: 可在多个hw项目中使用

## 快速开始

### 1. 包含头文件

```cpp
#include <async_mesh_processor.h>
#include <your_mesh_processor.h>
```

### 2. 创建异步处理器实例

```cpp
// 在main函数或主窗口中创建
AsyncMeshProcessor* asyncProcessor = new AsyncMeshProcessor(&window);
```

### 3. 设置处理函数

```cpp
YourMeshProcessor processor;  // 你的网格处理类

asyncProcessor->setProcessFunction(
    [&processor](const std::vector<QVector3D>& vertices,
                const std::vector<unsigned int>& indices) {
        // 这个函数会在独立线程中执行
        // 可以放心地执行耗时操作
        return processor.processOBJData(vertices, indices);
    });
```

### 4. 连接信号

```cpp
// 开始处理
QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingStarted,
    []() {
        qDebug() << "Processing started...";
        // 可以显示进度对话框
    });

// 处理完成
QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingFinished,
    [&window](const std::vector<QVector3D>& vertices,
             const std::vector<unsigned int>& indices) {
        qDebug() << "Processing completed!";
        window.updateMesh(vertices, indices);
    });

// 处理错误
QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingError,
    [](const QString& errorMessage) {
        qWarning() << "Error:" << errorMessage;
    });

// 进度更新（可选）
QObject::connect(asyncProcessor, &AsyncMeshProcessor::progressUpdated,
    [](int progress) {
        qDebug() << "Progress:" << progress << "%";
    });
```

### 5. 启动异步处理

```cpp
// 当加载OBJ文件后启动处理
QObject::connect(&window, &MainWindow::objLoaded,
    [asyncProcessor](const std::vector<QVector3D>& vertices,
                    const std::vector<unsigned int>& indices) {
        asyncProcessor->startProcessing(vertices, indices);
    });
```

## 完整示例 (hw1)

```cpp
#include <QApplication>
#include <QMessageBox>
#include <mainwindow.h>
#include <async_mesh_processor.h>
#include <mesh_processor.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.resize(800, 600);

    // 创建处理器
    MeshProcessor processor;
    AsyncMeshProcessor* asyncProcessor = new AsyncMeshProcessor(&window);

    // 设置处理函数
    asyncProcessor->setProcessFunction(
        [&processor](const std::vector<QVector3D>& vertices,
                    const std::vector<unsigned int>& indices) {
            return processor.processOBJData(vertices, indices);
        });

    // 连接OBJ加载事件
    QObject::connect(&window, &MainWindow::objLoaded,
        [asyncProcessor](const std::vector<QVector3D>& vertices,
                        const std::vector<unsigned int>& indices) {
            asyncProcessor->startProcessing(vertices, indices);
        });

    // 连接完成事件
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingFinished,
        [&window](const std::vector<QVector3D>& vertices,
                 const std::vector<unsigned int>& indices) {
            window.updateMesh(vertices, indices);
        });

    // 连接错误处理
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingError,
        [&window](const QString& errorMessage) {
            QMessageBox::warning(&window, "Error", errorMessage);
        });

    window.show();
    return app.exec();
}
```

## 在新的hw项目中使用

### hw2 示例

```cpp
// src/hw2/src/hw2.cpp
#include <QApplication>
#include <mainwindow.h>
#include <async_mesh_processor.h>
#include <hw2_mesh_processor.h>  // 你的hw2处理器

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.resize(800, 600);

    HW2MeshProcessor processor;  // 你的hw2特定处理器
    AsyncMeshProcessor* asyncProcessor = new AsyncMeshProcessor(&window);

    // 设置hw2的处理逻辑
    asyncProcessor->setProcessFunction(
        [&processor](const std::vector<QVector3D>& vertices,
                    const std::vector<unsigned int>& indices) {
            // hw2特定的处理逻辑
            return processor.doHW2Processing(vertices, indices);
        });

    // 其余连接代码相同...
    QObject::connect(&window, &MainWindow::objLoaded,
        [asyncProcessor](const std::vector<QVector3D>& vertices,
                        const std::vector<unsigned int>& indices) {
            asyncProcessor->startProcessing(vertices, indices);
        });

    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingFinished,
        [&window](const auto& v, const auto& i) {
            window.updateMesh(v, i);
        });

    window.show();
    return app.exec();
}
```

## API 参考

### AsyncMeshProcessor

#### 构造函数
```cpp
AsyncMeshProcessor(QObject *parent = nullptr)
```

#### 成员函数

```cpp
// 设置处理函数
void setProcessFunction(ProcessFunction func)

// 开始异步处理
void startProcessing(const std::vector<QVector3D>& vertices,
                    const std::vector<unsigned int>& indices)

// 检查是否正在处理
bool isProcessing() const

// 取消处理（注意：当前实现不会中断正在执行的操作）
void cancel()
```

#### 信号

```cpp
// 处理开始
void processingStarted()

// 处理完成
void processingFinished(const std::vector<QVector3D>& vertices,
                       const std::vector<unsigned int>& indices)

// 处理错误
void processingError(const QString& errorMessage)

// 进度更新
void progressUpdated(int progress)
```

## 注意事项

1. **线程安全**: 处理函数中不要直接操作Qt UI组件，使用信号-槽机制
2. **内存管理**: AsyncMeshProcessor使用父对象管理生命周期，通常将MainWindow作为父对象
3. **重复调用**: 如果正在处理中，再次调用`startProcessing`会被忽略
4. **异常处理**: 处理函数中的异常会被捕获并通过`processingError`信号发出

## 添加进度报告

如果需要在处理过程中报告进度，可以在处理器类中发送进度信号：

```cpp
// 在你的处理器类中
class MyProcessor {
public:
    std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
    processOBJData(const std::vector<QVector3D>& vertices,
                   const std::vector<unsigned int>& indices) {
        // 处理步骤1 - 30%
        doStep1();
        // 需要通过回调或其他机制报告进度
        
        // 处理步骤2 - 60%
        doStep2();
        
        // 处理步骤3 - 100%
        doStep3();
        
        return result;
    }
};
```

## 故障排除

### 问题：界面仍然无响应
- 确认处理函数真的在异步处理器中执行，而不是直接调用
- 检查是否在处理函数中调用了阻塞的UI操作

### 问题：处理结果没有更新到界面
- 确认连接了`processingFinished`信号
- 检查`updateMesh`是否正确调用

### 问题：编译错误找不到async_mesh_processor.h
- 确认viewer库已正确链接到你的hw项目
- 检查CMakeLists.txt中是否包含viewer依赖

## 扩展阅读

- Qt线程和QObject: https://doc.qt.io/qt-6/thread-basics.html
- 信号与槽机制: https://doc.qt.io/qt-6/signalsandslots.html
