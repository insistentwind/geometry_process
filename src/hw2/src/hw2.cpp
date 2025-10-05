#include <QApplication>
#include <mainwindow.h>
#include <halfedge.h>  // ʹ�ù���ļ��ο�
#include <iostream>

/**
 * @brief ʾ��������ϸ�ִ�����
 * �����չʾ��������µ���ҵ�и��ð�߽ṹ
 */
class MeshSubdivisionProcessor {
public:
    std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
    processOBJData(const std::vector<QVector3D>& vertices,
                  const std::vector<unsigned int>& indices) {
        
        // ת���������ݵ���߽ṹ
        std::vector<Eigen::Vector3d> eigenVerts;
        eigenVerts.reserve(vertices.size());
        for (const auto& v : vertices) {
            eigenVerts.emplace_back(v.x(), v.y(), v.z());
        }

        std::vector<std::vector<int>> faces;
        for (size_t i = 0; i < indices.size(); i += 3) {
            std::vector<int> face = {
                static_cast<int>(indices[i]),
                static_cast<int>(indices[i + 1]),
                static_cast<int>(indices[i + 2])
            };
            faces.push_back(face);
        }

        // ������߽ṹ
        mesh.buildFromOBJ(eigenVerts, faces);
        
        std::cout << "Original mesh: " << mesh.getVertexCount() << " vertices, " 
                  << mesh.getFaceCount() << " faces" << std::endl;

        // ��֤��߽ṹ
        if (!mesh.isValid()) {
            std::cerr << "Error: Invalid half-edge mesh!" << std::endl;
            return {vertices, indices}; // ����ԭʼ����
        }

        // ִ������ϸ�֣��������ʵ��Catmull-Clarkϸ�ֵ��㷨��
        performSubdivision();

        // ת����QVector3D��ʽ
        std::vector<QVector3D> processedVertices;
        std::vector<unsigned int> processedIndices;

        processedVertices.reserve(mesh.vertices.size());
        for (const auto& vertex : mesh.vertices) {
            processedVertices.emplace_back(
                static_cast<float>(vertex->position.x()),
                static_cast<float>(vertex->position.y()),
                static_cast<float>(vertex->position.z())
            );
        }

        for (const auto& face : mesh.faces) {
            geometry::HalfEdge* he = face->halfEdge;
            do {
                processedIndices.push_back(static_cast<unsigned int>(he->vertex->index));
                he = he->next;
            } while (he != face->halfEdge);
        }

        std::cout << "Processed mesh: " << processedVertices.size() << " vertices, " 
                  << processedIndices.size() / 3 << " faces" << std::endl;

        return {processedVertices, processedIndices};
    }

private:
    geometry::HalfEdgeMesh mesh;

    void performSubdivision() {
        // �������ʵ�ָ���ϸ���㷨
        // ���磺Loopϸ�֡�Catmull-Clarkϸ�ֵ�
        
        // ʾ�����򵥵Ķ���ƽ��
        std::vector<Eigen::Vector3d> newPositions(mesh.vertices.size());
        
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            // �����ھӶ����ƽ��λ��
            Eigen::Vector3d avgPos = mesh.vertices[i]->position;
            int neighborCount = 1;
            
            // �����ھӶ���
            geometry::HalfEdge* he = mesh.vertices[i]->halfEdge;
            if (he) {
                geometry::HalfEdge* current = he;
                do {
                    if (current->pair && current->pair->vertex) {
                        avgPos += current->pair->vertex->position;
                        neighborCount++;
                    }
                    current = current->pair ? current->pair->next : nullptr;
                } while (current && current != he);
            }
            
            newPositions[i] = avgPos / neighborCount;
        }
        
        // Ӧ����λ��
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            mesh.vertices[i]->position = newPositions[i];
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.resize(800, 600);

    // ��������ϸ�ִ�����
    MeshSubdivisionProcessor processor;

    // �����źźͲ�
    QObject::connect(&window, &MainWindow::objLoaded,
        [&processor, &window](const std::vector<QVector3D>& vertices,
                            const std::vector<unsigned int>& indices) {
            // ��������
            auto [processedVertices, processedIndices] = 
                processor.processOBJData(vertices, indices);

            // ������ʾ
            window.updateMesh(processedVertices, processedIndices);
        });

    window.show();
    return app.exec();
}