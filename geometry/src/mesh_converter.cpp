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
        qVertices.emplace_back(
            static_cast<float>(v.x()),
            static_cast<float>(v.y()),
            static_cast<float>(v.z())
        );
    }
    return qVertices;
}

std::vector<std::vector<int>> MeshConverter::convertIndicesToFaces(const std::vector<unsigned int>& indices) {
    std::vector<std::vector<int>> faces;
    // ��������������������ÿ3����������һ����
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 < indices.size()) {
            std::vector<int> face = {
                static_cast<int>(indices[i]),
                static_cast<int>(indices[i + 1]),
                static_cast<int>(indices[i + 2])
            };
            faces.push_back(face);
        }
    }
    return faces;
}
// �����Ǵ�obj����������ݽṹ
void MeshConverter::buildMeshFromQtData(HalfEdgeMesh& mesh,
                                        const std::vector<QVector3D>& qVertices,
                                        const std::vector<unsigned int>& indices) {
    // ת��Qt��ʽ��Eigen��ʽ
    auto eigenVerts = convertQtToEigen(qVertices);
    
    // ת���������鵽��ṹ
    auto facesData = convertIndicesToFaces(indices);
    
    // ���ð�������buildFromOBJ����
    mesh.buildFromOBJ(eigenVerts, facesData);
}

std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
MeshConverter::convertMeshToQtData(const HalfEdgeMesh& mesh) {
    std::vector<QVector3D> qVertices;
    std::vector<unsigned int> indices;

    // ת���������ݣ�Eigen::Vector3d -> QVector3D
    qVertices.reserve(mesh.vertices.size());
    for (const auto& vertex : mesh.vertices) {
        qVertices.emplace_back(
            static_cast<float>(vertex->position.x()),
            static_cast<float>(vertex->position.y()),
            static_cast<float>(vertex->position.z())
        );
    }

    // ת����������ͨ����������ؽ���������
    for (const auto& face : mesh.faces) {
        HalfEdge* he = face->halfEdge;
        do {
            indices.push_back(static_cast<unsigned int>(he->vertex->index));
            he = he->next;
        } while (he != face->halfEdge);
    }

    return {qVertices, indices};
}

} // namespace geometry
