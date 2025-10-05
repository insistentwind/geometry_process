#include <QApplication>
#include <mainwindow.h>
#include <mesh_processor.h>
#include <iostream>

/**
 * @brief ������ - ������ڵ�
 * רע��QtӦ�ó���ĳ�ʼ�����¼�����
 * ��Ҫ�������裺
 * 1. ����QtӦ�ó���
 * 2. ���������ں���������
 * 3. �����ź�-�����ӣ���ӦOBJ�ļ������¼�
 * 4. ����Ӧ�ó�����ѭ��
 */
int main(int argc, char *argv[])
{
    // ����1����ʼ��QtӦ�ó���
    QApplication app(argc, argv);
    MainWindow window;
    window.resize(800, 600);

    // ����2��������������ʵ��
    // ����ʹ�ö�����MeshProcessor��
    MeshProcessor processor;

    // ����3�������¼���Ӧ����
    // �������ڼ���OBJ�ļ�ʱ���Զ���������������
    QObject::connect(&window, &MainWindow::objLoaded,
        [&processor, &window](const std::vector<QVector3D>& vertices,
                            const std::vector<unsigned int>& indices) {
            std::cout << "OBJ file loaded, starting mesh processing..." << std::endl;
            
            // ���ô�����������ص���������
            auto [processedVertices, processedIndices] = 
                processor.processOBJData(vertices, indices);

            std::cout << "Mesh processing completed. Updating display..." << std::endl;
            
            // ������ʾ
            window.updateMesh(processedVertices, processedIndices);
        });

    // ����4����ʾ���ڲ�����Ӧ�ó����¼�ѭ��
    window.show();
    return app.exec();
}