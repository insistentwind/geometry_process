#include <QApplication>
#include <QMessageBox>
#include <mainwindow.h>
#include <async_mesh_processor.h>
#include <mesh_processor.h>
#include <iostream>
#include <Eigen/Sparse>
/**
 * @brief 主程序 - 改进的入口点
 * 专注于Qt应用程序的初始化和事件循环
 * 主要功能部分：
 * 1. 创建Qt应用程序
 * 2. 创建主窗口和异步处理器
 * 3. 连接信号-槽链接，响应OBJ文件加载和处理按钮事件
 * 4. 使用异步处理避免界面无响应
 */
int main(int argc, char *argv[])
{
    // 步骤1：初始化Qt应用程序
    QApplication app(argc, argv);
    MainWindow window;
    window.resize(800, 600);

    // 步骤2：创建网格处理实例和异步处理器
    MeshProcessor processor;
    AsyncMeshProcessor* asyncProcessor = new AsyncMeshProcessor(&window);

    // 设置异步处理函数
    asyncProcessor->setProcessFunction(
        [&processor](const std::vector<QVector3D>& vertices,
                    const std::vector<unsigned int>& indices) {
            // 这个函数会在独立线程中执行，不会阻塞UI
            std::cout << "Processing mesh in worker thread..." << std::endl;
            return processor.processOBJData(vertices, indices);
        });

    // 步骤3：连接事件响应链路
    
    // 3.1 当窗口加载OBJ文件时，启动异步处理
    QObject::connect(&window, &MainWindow::objLoaded,
        [asyncProcessor](const std::vector<QVector3D>& vertices,
                         const std::vector<unsigned int>& indices) {
            std::cout << "OBJ file loaded, starting async processing..." << std::endl;
            asyncProcessor->startProcessing(vertices, indices);
        });

    // 处理开始时的反馈
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingStarted,
        []() {
            std::cout << "Processing started - UI remains responsive" << std::endl;
        });

    // 处理完成时更新显示
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingFinished,
        [&window, &processor](const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices) {
            std::cout << "Async mesh processing completed. Updating display..." << std::endl;
            auto colors = processor.extractColors();
            window.updateMeshWithColors(vertices, indices, colors);
            auto mstEdges = processor.extractMSTEdges();
            // 通过 glWidget 接口更新 MST 线，需要访问 glWidget，可在 MainWindow 提供转发
            // 这里直接使用 dynamic_cast 找到子控件（简单方式）
            auto gl = window.findChild<GLWidget*>();
            if(gl){ gl->updateMSTEdges(mstEdges); }
        });

    // 处理错误
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingError,
        [&window](const QString& errorMessage) {
            std::cerr << "Processing error: " << errorMessage.toStdString() << std::endl;
            QMessageBox::warning(&window, "Processing Error", 
                               "Mesh processing failed: " + errorMessage);
        });

    // 步骤4：显示窗口并启动应用程序事件循环
    window.show();
    return app.exec();
}