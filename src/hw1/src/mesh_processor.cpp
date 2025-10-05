#include "mesh_processor.h"
#include <iostream>

std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
MeshProcessor::processOBJData(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices) {
    
    // ����1����Qt��QVector3D��ʽת��ΪEigen::Vector3d��ʽ
    auto eigenVerts = convertToEigen(vertices);

    // ����2������������������ת��Ϊ��ṹ
    auto faces = convertToFaces(indices);

    // ����3�������������ṹ
    // ���Ǻ��Ĳ��裬���򵥵Ķ���-���ʾת��Ϊ�����İ�����ݽṹ
    mesh.buildFromOBJ(eigenVerts, faces);

    // ����4����֤��߽ṹ����ȷ��
    // ȷ�������İ������������������ȷ��
    if (!mesh.isValid()) {
        std::cerr << "Warning: Generated half-edge mesh is invalid!" << std::endl;
    }

    // ����5��ִ��ʵ�ʵļ��δ������
    // ������Խ��и��������㷨��ƽ�����򻯡�ϸ�ֵ�
    processGeometry();

    // ����6���������İ�߽ṹת����Qt���õĸ�ʽ
    return convertFromMesh();
}

void MeshProcessor::processGeometry() {
    // ʾ�������򵥵Ķ���λ��
    // ��ʵ��Ӧ���У�����Ӧ��������������㷨
    for (auto& vertex : mesh.vertices) {
        // ʾ������ÿ���������䷨�߷����ƶ�һС�ξ���
        // ����ֻ��ʾ����������滻����Ĵ����߼�
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
    // ��������������������ÿ3����������һ����
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

    // ת���������ݣ�Eigen::Vector3d -> QVector3D
    processedVertices.reserve(mesh.vertices.size());
    for (const auto& vertex : mesh.vertices) {
        processedVertices.emplace_back(
            static_cast<float>(vertex->position.x()),
            static_cast<float>(vertex->position.y()),
            static_cast<float>(vertex->position.z())
        );
    }

    // ת����������ͨ����������ؽ���������
    for (const auto& face : mesh.faces) {
        geometry::HalfEdge* he = face->halfEdge;
        do {
            processedIndices.push_back(static_cast<unsigned int>(he->vertex->index));
            he = he->next;
        } while (he != face->halfEdge);
    }

    return {processedVertices, processedIndices};
}