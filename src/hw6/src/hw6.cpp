#include <QApplication>
#include <QMessageBox>
#include <mainwindow.h>
#include <async_mesh_processor.h>
#include <mesh_processor.h>
#include <mesh_converter.h>
#include <iostream>
#include <Eigen/Sparse>

/**
 * @brief HW6������ - ARAP���ν���
 * 
 * �������̣�
 * 1. �û���OBJģ��
 * 2. ���"enter ARAP"��ť����ARAPģʽ
 * 3. ������������Ϊfixed���̶��㣩���ɶ�ε��ѡ����
 * 4. ���"clear fixed"������й̶�������ѡ��
 * 5. ѡ�ù̶��������϶�������Ϊhandle���б���
 * 6. �϶�ʱʵʱ����ARAP�㷨���б��ձ���
 * 7. ���"exit ARAP"�˳�ARAPģʽ���ָ������������
 */
int main(int argc, char* argv[])
{
    // ����1����ʼ��QtӦ�ó���
    QApplication app(argc, argv);
    MainWindow window;
    window.resize(1000, 800);

    // ����2������������ʵ��
    MeshProcessor processor;   // �����첽mesh����ȥ��ȣ�
    MeshProcessor arapProcessor;    // ר����ARAP����
    AsyncMeshProcessor* asyncProcessor = new AsyncMeshProcessor(&window);

  // �����첽����������ͨȥ��Ȳ�����
    asyncProcessor->setProcessFunction(
        [&processor](const std::vector<QVector3D>& vertices,
  const std::vector<unsigned int>& indices) {
         std::cout << "Processing mesh in worker thread..." << std::endl;
        return processor.processOBJData(vertices, indices);
        });

    // ����3������ARAP�ص�������GLWidget
    
    // 3.1 ��ʼARAP�Ự�ص������浱ǰλ��Ϊold_position�����fixed���
    window.findChild<GLWidget*>()->arapBeginCallback = [&arapProcessor, &window]() {
        std::cout << "[ARAP] Begin ARAP session - saving current mesh state" << std::endl;
        // ��GLWidget��ȡ��ǰmesh���ݹ�����߽ṹ
   auto verts = window.findChild<GLWidget*>()->getVertices();
    auto inds = window.findChild<GLWidget*>()->getIndices();
        geometry::MeshConverter::buildMeshFromQtData(arapProcessor.getMesh(), verts, inds);
        arapProcessor.beginArapSession();
    };

    // 3.2 ���ù̶�����ص�
    window.findChild<GLWidget*>()->arapSetFixedCallback = [&arapProcessor](int index, bool fixed) {
     std::cout << "[ARAP] Set vertex " << index << " as " << (fixed ? "FIXED" : "FREE") << std::endl;
 arapProcessor.setFixedVertex(index, fixed);
    };

    // 3.3 ������й̶�����ص�
    window.findChild<GLWidget*>()->arapClearFixedCallback = [&arapProcessor]() {
        std::cout << "[ARAP] Clear all fixed vertices" << std::endl;
    arapProcessor.clearFixedVertices();
    };

    // 3.4 �϶�handle����ص���ִ��ARAP���β������µ�mesh
window.findChild<GLWidget*>()->arapDragCallback = [&arapProcessor](int handleIndex, const QVector3D& newPos) {
        std::cout << "[ARAP] Dragging handle vertex " << handleIndex 
         << " to (" << newPos.x() << ", " << newPos.y() << ", " << newPos.z() << ")" << std::endl;
        return arapProcessor.applyArapDrag(handleIndex, newPos);
    };

    // ����4��������ͨmesh�����¼���Ӧ��·

    // 4.1 �����ڼ���OBJ�ļ�ʱ����ʼ��ARAP������
 QObject::connect(&window, &MainWindow::objLoaded,
        [asyncProcessor, &arapProcessor](const std::vector<QVector3D>& vertices,
        const std::vector<unsigned int>& indices) {
std::cout << "OBJ file loaded with " << vertices.size() << " vertices" << std::endl;
     // ��ʼ��ARAP��������mesh
            geometry::MeshConverter::buildMeshFromQtData(
           const_cast<geometry::HalfEdgeMesh&>(arapProcessor.getMesh()), 
        vertices, indices);
   
            // ��ѡ�������첽ȥ�봦��
            // asyncProcessor->startProcessing(vertices, indices);
        });

    // ����ʼʱ�ķ���
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingStarted,
        []() {
  std::cout << "Processing started - UI remains responsive" << std::endl;
    });

    // �������ʱ������ʾ
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingFinished,
        [&window](const std::vector<QVector3D>& vertices,
         const std::vector<unsigned int>& indices) {
            std::cout << "Async mesh processing completed. Updating display..." << std::endl;
      window.updateMesh(vertices, indices);
        });

    // �������
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingError,
        [&window](const QString& errorMessage) {
 std::cerr << "Processing error: " << errorMessage.toStdString() << std::endl;
      QMessageBox::warning(&window, "Processing Error",
        "Mesh processing failed: " + errorMessage);
        });

    // ����5����ʾ���ڲ�����Ӧ�ó����¼�ѭ��
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