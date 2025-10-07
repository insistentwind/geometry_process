#include <QApplication>
#include <QMessageBox>
#include <mainwindow.h>
#include <async_mesh_processor.h>
#include <mesh_processor.h>
#include <iostream>
#include <Eigen/Sparse>
/**
 * @brief ������ - �Ľ�����ڵ�
 * רע��QtӦ�ó���ĳ�ʼ�����¼�ѭ��
 * ��Ҫ���ܲ��֣�
 * 1. ����QtӦ�ó���
 * 2. ���������ں��첽������
 * 3. �����ź�-�����ӣ���ӦOBJ�ļ����غʹ���ť�¼�
 * 4. ʹ���첽��������������Ӧ
 */
int main(int argc, char *argv[])
{
    // ����1����ʼ��QtӦ�ó���
    QApplication app(argc, argv);
    MainWindow window;
    window.resize(800, 600);

    // ����2������������ʵ�����첽������
    MeshProcessor processor;
    AsyncMeshProcessor* asyncProcessor = new AsyncMeshProcessor(&window);

    // �����첽������
    asyncProcessor->setProcessFunction(
        [&processor](const std::vector<QVector3D>& vertices,
                    const std::vector<unsigned int>& indices) {
            // ����������ڶ����߳���ִ�У���������UI
            std::cout << "Processing mesh in worker thread..." << std::endl;
            return processor.processOBJData(vertices, indices);
        });

    // ����3�������¼���Ӧ��·
    
    // 3.1 �����ڼ���OBJ�ļ�ʱ�������첽����
    QObject::connect(&window, &MainWindow::objLoaded,
        [asyncProcessor](const std::vector<QVector3D>& vertices,
                         const std::vector<unsigned int>& indices) {
            std::cout << "OBJ file loaded, starting async processing..." << std::endl;
            asyncProcessor->startProcessing(vertices, indices);
        });

    // ����ʼʱ�ķ���
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingStarted,
        []() {
            std::cout << "Processing started - UI remains responsive" << std::endl;
        });

    // �������ʱ������ʾ
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingFinished,
        [&window, &processor](const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices) {
            std::cout << "Async mesh processing completed. Updating display..." << std::endl;
            auto colors = processor.extractColors();
            window.updateMeshWithColors(vertices, indices, colors);
            auto mstEdges = processor.extractMSTEdges();
            // ͨ�� glWidget �ӿڸ��� MST �ߣ���Ҫ���� glWidget������ MainWindow �ṩת��
            // ����ֱ��ʹ�� dynamic_cast �ҵ��ӿؼ����򵥷�ʽ��
            auto gl = window.findChild<GLWidget*>();
            if(gl){ gl->updateMSTEdges(mstEdges); }
        });

    // �������
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingError,
        [&window](const QString& errorMessage) {
            std::cerr << "Processing error: " << errorMessage.toStdString() << std::endl;
            QMessageBox::warning(&window, "Processing Error", 
                               "Mesh processing failed: " + errorMessage);
        });

    // ����4����ʾ���ڲ�����Ӧ�ó����¼�ѭ��
    window.show();
    return app.exec();
}