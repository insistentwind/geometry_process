#include <QApplication>
#include <mainwindow.h>
#include <mesh_processor.h>
#include <iostream>

/**
 * @brief 主函数 - 程序入口点
 * 专注于Qt应用程序的初始化和事件处理
 * 主要操作步骤：
 * 1. 创建Qt应用程序
 * 2. 创建主窗口和网格处理器
 * 3. 设置信号-槽连接，响应OBJ文件加载事件
 * 4. 启动应用程序主循环
 */
int main(int argc, char *argv[])
{
    // 步骤1：初始化Qt应用程序
    QApplication app(argc, argv);
    MainWindow window;
    window.resize(800, 600);

    // 步骤2：创建网格处理器实例
    // 现在使用独立的MeshProcessor类
    MeshProcessor processor;

    // 步骤3：设置事件响应机制
    // 当主窗口加载OBJ文件时，自动触发网格处理流程
    QObject::connect(&window, &MainWindow::objLoaded,
        [&processor, &window](const std::vector<QVector3D>& vertices,
                            const std::vector<unsigned int>& indices) {
            std::cout << "OBJ file loaded, starting mesh processing..." << std::endl;
            
            // 调用处理器处理加载的网格数据
            auto [processedVertices, processedIndices] = 
                processor.processOBJData(vertices, indices);

            std::cout << "Mesh processing completed. Updating display..." << std::endl;
            
            // 更新显示
            window.updateMesh(processedVertices, processedIndices);
        });

    // 步骤4：显示窗口并启动应用程序事件循环
    window.show();
    return app.exec();
}