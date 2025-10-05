#include <QApplication>
#include <mainwindow.h>
#include <halfedge.h>  // 使用共享的几何库
#include <iostream>

/**
 * @brief 示例：网格细分处理器
 * 这个类展示了如何在新的作业中复用半边结构
 */
class MeshSubdivisionProcessor {
public:
    std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
    processOBJData(const std::vector<QVector3D>& vertices,
                  const std::vector<unsigned int>& indices) {
        
        // 转换输入数据到半边结构
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

        // 构建半边结构
        mesh.buildFromOBJ(eigenVerts, faces);
        
        std::cout << "Original mesh: " << mesh.getVertexCount() << " vertices, " 
                  << mesh.getFaceCount() << " faces" << std::endl;

        // 验证半边结构
        if (!mesh.isValid()) {
            std::cerr << "Error: Invalid half-edge mesh!" << std::endl;
            return {vertices, indices}; // 返回原始数据
        }

        // 执行网格细分（这里可以实现Catmull-Clark细分等算法）
        performSubdivision();

        // 转换回QVector3D格式
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
        // 这里可以实现各种细分算法
        // 例如：Loop细分、Catmull-Clark细分等
        
        // 示例：简单的顶点平滑
        std::vector<Eigen::Vector3d> newPositions(mesh.vertices.size());
        
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            // 计算邻居顶点的平均位置
            Eigen::Vector3d avgPos = mesh.vertices[i]->position;
            int neighborCount = 1;
            
            // 遍历邻居顶点
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
        
        // 应用新位置
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

    // 创建网格细分处理器
    MeshSubdivisionProcessor processor;

    // 连接信号和槽
    QObject::connect(&window, &MainWindow::objLoaded,
        [&processor, &window](const std::vector<QVector3D>& vertices,
                            const std::vector<unsigned int>& indices) {
            // 处理数据
            auto [processedVertices, processedIndices] = 
                processor.processOBJData(vertices, indices);

            // 更新显示
            window.updateMesh(processedVertices, processedIndices);
        });

    window.show();
    return app.exec();
}