#include "mesh_converter.h"

namespace geometry {

std::vector<Eigen::Vector3d> MeshConverter::convertQtToEigen(const std::vector<QVector3D>& qVertices) {
    std::vector<Eigen::Vector3d> eigenVerts;
    eigenVerts.reserve(qVertices.size());
    for (const auto& v : qVertices) {
        eigenVerts.emplace_back(v.x(), v.y(), v.z());
    }
    return eigenVerts;
}

std::vector<QVector3D> MeshConverter::convertEigenToQt(const std::vector<Eigen::Vector3d>& eigenVerts) {
    std::vector<QVector3D> qVertices;
    qVertices.reserve(eigenVerts.size());
    for (const auto& v : eigenVerts) {
        qVertices.emplace_back(static_cast<float>(v.x()),
                               static_cast<float>(v.y()),
                               static_cast<float>(v.z()));
    }
    return qVertices;
}

std::vector<std::vector<int>> MeshConverter::convertIndicesToFaces(const std::vector<unsigned int>& indices) {
    std::vector<std::vector<int>> faces;
    faces.reserve(indices.size() / 3);

    // 每3个索引组成一个三角面
    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        faces.push_back({ static_cast<int>(indices[i]),
                          static_cast<int>(indices[i + 1]),
                          static_cast<int>(indices[i + 2]) });
    }
    return faces;
}

void MeshConverter::buildMeshFromQtData(HalfEdgeMesh& mesh,
                                        const std::vector<QVector3D>& qVertices,
                                        const std::vector<unsigned int>& indices) {
    // 1. 顶点格式转换
    auto eigenVerts = convertQtToEigen(qVertices);
    // 2. 构造面索引列表
    auto facesData  = convertIndicesToFaces(indices);
    // 3. 构建半边网格
    mesh.buildFromOBJ(eigenVerts, facesData);
}

std::pair<std::vector<QVector3D>, std::vector<unsigned int>>
MeshConverter::convertMeshToQtData(const HalfEdgeMesh& mesh) {
    std::vector<QVector3D> qVertices;
    std::vector<unsigned int> indices;

    // 顶点数据复制
    qVertices.reserve(mesh.vertices.size());
    for (const auto& vertex : mesh.vertices) {
        qVertices.emplace_back(static_cast<float>(vertex->position.x()),
                               static_cast<float>(vertex->position.y()),
                               static_cast<float>(vertex->position.z()));
    }

    // 面 -> 索引 (假设都是简单多边形(目前为三角形))
    indices.reserve(mesh.faces.size() * 3);
    for (const auto& face : mesh.faces) {
        HalfEdge* he = face->halfEdge;
        if (!he) continue;
        do {
            indices.push_back(static_cast<unsigned int>(he->vertex->index));
            he = he->next;
        } while (he && he != face->halfEdge);
    }

    return { qVertices, indices };
}

} // namespace geometry
