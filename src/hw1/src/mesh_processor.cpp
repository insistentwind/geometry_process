#include "mesh_processor.h"
#include <mesh_converter.h>
#include <iostream>

std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
MeshProcessor::processOBJData(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices) {
    
    // ����1��ʹ��geometryģ���MeshConverter�����������
    geometry::MeshConverter::buildMeshFromQtData(mesh, vertices, indices);

    // ����2����֤��߽ṹ����ȷ��
    if (!mesh.isValid()) {
        std::cerr << "Warning: Generated half-edge mesh is invalid!" << std::endl;
    }

    // ����3��ִ��ʵ�ʵļ��δ��������������Ҫ�Լ�ʵ�ֵĲ��֣�
    processGeometry();

    // ����4��ʹ��geometryģ���MeshConverter�����ת��Qt��ʽ
    return geometry::MeshConverter::convertMeshToQtData(mesh);
}

void MeshProcessor::processGeometry() {
    // TODO: ������ʵ������������㷨
    // ʾ�����򵥵Ķ���λ��
    for (auto& vertex : mesh.vertices) {
        // ʾ������ÿ��������ĳ�������ƶ�һС�ξ���
        // ������滻�����Լ��Ĵ����߼�
        vertex->position += Eigen::Vector3d(0.1, 0.1, 0.0);
    }
}