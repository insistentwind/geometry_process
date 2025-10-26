#include <QApplication>
#include <QMessageBox>
#include <mainwindow.h>
#include <async_mesh_processor.h>
#include <mesh_processor.h>
#include <mesh_converter.h>
#include <iostream>
#include <Eigen/Sparse>

/**
 * @brief HW6主程序 - ARAP变形交互
 * 
 * 工作流程：
 * 1. 用户打开OBJ模型
 * 2. 点击"enter ARAP"按钮进入ARAP模式
 * 3. 左键点击顶点设为fixed（固定点），可多次点击选择多个
 * 4. 点击"clear fixed"清除所有固定点重新选择
 * 5. 选好固定点后，左键拖动顶点作为handle进行变形
 * 6. 拖动时实时调用ARAP算法进行保刚变形
 * 7. 点击"exit ARAP"退出ARAP模式，恢复正常相机交互
 */
int main(int argc, char* argv[])
{
    // 步骤1：初始化Qt应用程序
    QApplication app(argc, argv);
    MainWindow window;
    window.resize(1000, 800);

    // 步骤2：创建网格处理实例
    MeshProcessor processor;   // 用于异步mesh处理（去噪等）
    MeshProcessor arapProcessor;    // 专用于ARAP交互
    AsyncMeshProcessor* asyncProcessor = new AsyncMeshProcessor(&window);

  // 设置异步处理函数（普通去噪等操作）
    asyncProcessor->setProcessFunction(
        [&processor](const std::vector<QVector3D>& vertices,
  const std::vector<unsigned int>& indices) {
         std::cout << "Processing mesh in worker thread..." << std::endl;
        return processor.processOBJData(vertices, indices);
        });

    // 步骤3：设置ARAP回调函数到GLWidget
    
    // 3.1 开始ARAP会话回调：保存当前位置为old_position并清空fixed标记
    window.findChild<GLWidget*>()->arapBeginCallback = [&arapProcessor, &window]() {
        std::cout << "[ARAP] Begin ARAP session - saving current mesh state" << std::endl;
        // 从GLWidget获取当前mesh数据构建半边结构
   auto verts = window.findChild<GLWidget*>()->getVertices();
    auto inds = window.findChild<GLWidget*>()->getIndices();
        geometry::MeshConverter::buildMeshFromQtData(arapProcessor.getMesh(), verts, inds);
        arapProcessor.beginArapSession();
    };

    // 3.2 设置固定顶点回调
    window.findChild<GLWidget*>()->arapSetFixedCallback = [&arapProcessor](int index, bool fixed) {
     std::cout << "[ARAP] Set vertex " << index << " as " << (fixed ? "FIXED" : "FREE") << std::endl;
 arapProcessor.setFixedVertex(index, fixed);
    };

    // 3.3 清除所有固定顶点回调
    window.findChild<GLWidget*>()->arapClearFixedCallback = [&arapProcessor]() {
        std::cout << "[ARAP] Clear all fixed vertices" << std::endl;
    arapProcessor.clearFixedVertices();
    };

    // 3.4 拖动handle顶点回调：执行ARAP变形并返回新的mesh
window.findChild<GLWidget*>()->arapDragCallback = [&arapProcessor](int handleIndex, const QVector3D& newPos) {
        std::cout << "[ARAP] Dragging handle vertex " << handleIndex 
         << " to (" << newPos.x() << ", " << newPos.y() << ", " << newPos.z() << ")" << std::endl;
        return arapProcessor.applyArapDrag(handleIndex, newPos);
    };

    // 步骤4：连接普通mesh处理事件响应链路

    // 4.1 当窗口加载OBJ文件时，初始化ARAP处理器
 QObject::connect(&window, &MainWindow::objLoaded,
        [asyncProcessor, &arapProcessor](const std::vector<QVector3D>& vertices,
        const std::vector<unsigned int>& indices) {
std::cout << "OBJ file loaded with " << vertices.size() << " vertices" << std::endl;
     // 初始化ARAP处理器的mesh
            geometry::MeshConverter::buildMeshFromQtData(
           const_cast<geometry::HalfEdgeMesh&>(arapProcessor.getMesh()), 
        vertices, indices);
   
            // 可选：启动异步去噪处理
            // asyncProcessor->startProcessing(vertices, indices);
        });

    // 处理开始时的反馈
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingStarted,
        []() {
  std::cout << "Processing started - UI remains responsive" << std::endl;
    });

    // 处理完成时更新显示
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingFinished,
        [&window](const std::vector<QVector3D>& vertices,
         const std::vector<unsigned int>& indices) {
            std::cout << "Async mesh processing completed. Updating display..." << std::endl;
      window.updateMesh(vertices, indices);
        });

    // 处理错误
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingError,
        [&window](const QString& errorMessage) {
 std::cerr << "Processing error: " << errorMessage.toStdString() << std::endl;
      QMessageBox::warning(&window, "Processing Error",
        "Mesh processing failed: " + errorMessage);
        });

    // 步骤5：显示窗口并启动应用程序事件循环
    window.show();
    
    std::cout << "\n=== ARAP Interaction Guide ===" << std::endl;
    std::cout << "1. Open an OBJ model (File -> Open)" << std::endl;
std::cout << "2. Click 'enter ARAP' button" << std::endl;
    std::cout << "3. Left-click vertices to mark as FIXED (can select multiple)" << std::endl;
    std::cout << "4. Click 'clear fixed' to reset selection" << std::endl;
    std::cout << "5. Left-click and DRAG a vertex as HANDLE to deform (model won't rotate)" << std::endl;
    std::cout << "6. Click 'exit ARAP' to return to normal mode" << std::endl;
    std::cout << "================================\n" << std::endl;
 
    return app.exec();
}