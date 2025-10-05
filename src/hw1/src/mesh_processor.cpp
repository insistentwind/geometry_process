#include "mesh_processor.h"
#include <iostream>

std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
MeshProcessor::processOBJData(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices) {
    
    // 步骤1：将Qt的QVector3D格式转换为Eigen::Vector3d格式
    auto eigenVerts = convertToEigen(vertices);

    // 步骤2：将连续的索引数组转换为面结构
    auto faces = convertToFaces(indices);

    // 步骤3：构建半边网格结构
    // 这是核心步骤，将简单的顶点-面表示转换为完整的半边数据结构
    mesh.buildFromOBJ(eigenVerts, faces);

    // 步骤4：验证半边结构的正确性
    // 确保构建的半边网格在拓扑上是正确的
    if (!mesh.isValid()) {
        std::cerr << "Warning: Generated half-edge mesh is invalid!" << std::endl;
    }

    // 步骤5：执行实际的几何处理操作
    // 这里可以进行各种网格算法：平滑、简化、细分等
    processGeometry();

    // 步骤6：将处理后的半边结构转换回Qt可用的格式
    return convertFromMesh();
}

void MeshProcessor::processGeometry() {
    // 示例处理：简单的顶点位移
    // 在实际应用中，这里应该是你的网格处理算法
    for (auto& vertex : mesh.vertices) {
        // 示例：将每个顶点向其法线方向移动一小段距离
        // 这里只是示例，你可以替换成你的处理逻辑
        vertex->position += Eigen::Vector3d(0.1, 0.1, 0.0);
    }
}

std::vector<Eigen::Vector3d> MeshProcessor::convertToEigen(const std::vector<QVector3D>& vertices) {
    std::vector<Eigen::Vector3d> eigenVerts;
    eigenVerts.reserve(vertices.size());
    for (const auto& v : vertices) {
        eigenVerts.emplace_back(v.x(), v.y(), v.z());
    }
    return eigenVerts;
}

std::vector<std::vector<int>> MeshProcessor::convertToFaces(const std::vector<unsigned int>& indices) {
    std::vector<std::vector<int>> faces;
    // 假设输入是三角形网格，每3个索引构成一个面
    for (size_t i = 0; i < indices.size(); i += 3) {
        std::vector<int> face = {
            static_cast<int>(indices[i]),
            static_cast<int>(indices[i + 1]),
            static_cast<int>(indices[i + 2])
        };
        faces.push_back(face);
    }
    return faces;
}

std::pair<std::vector<QVector3D>, std::vector<unsigned int>> MeshProcessor::convertFromMesh() {
    std::vector<QVector3D> processedVertices;
    std::vector<unsigned int> processedIndices;

    // 转换顶点数据：Eigen::Vector3d -> QVector3D
    processedVertices.reserve(mesh.vertices.size());
    for (const auto& vertex : mesh.vertices) {
        processedVertices.emplace_back(
            static_cast<float>(vertex->position.x()),
            static_cast<float>(vertex->position.y()),
            static_cast<float>(vertex->position.z())
        );
    }

    // 转换面索引：通过遍历半边重建索引数组
    for (const auto& face : mesh.faces) {
        geometry::HalfEdge* he = face->halfEdge;
        do {
            processedIndices.push_back(static_cast<unsigned int>(he->vertex->index));
            he = he->next;
        } while (he != face->halfEdge);
    }

    return {processedVertices, processedIndices};
}