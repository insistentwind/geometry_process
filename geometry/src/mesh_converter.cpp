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

    // ÿ3���������һ��������
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
    // 1. �����ʽת��
    auto eigenVerts = convertQtToEigen(qVertices);
    // 2. �����������б�
    auto facesData  = convertIndicesToFaces(indices);
    // 3. �����������
    mesh.buildFromOBJ(eigenVerts, facesData);
}

std::pair<std::vector<QVector3D>, std::vector<unsigned int>>
MeshConverter::convertMeshToQtData(const HalfEdgeMesh& mesh) {
    std::vector<QVector3D> qVertices;
    std::vector<unsigned int> indices;

    // �������ݸ���
    qVertices.reserve(mesh.vertices.size());
    for (const auto& vertex : mesh.vertices) {
        qVertices.emplace_back(static_cast<float>(vertex->position.x()),
                               static_cast<float>(vertex->position.y()),
                               static_cast<float>(vertex->position.z()));
    }

    // �� -> ���� (���趼�Ǽ򵥶����(ĿǰΪ������))
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
