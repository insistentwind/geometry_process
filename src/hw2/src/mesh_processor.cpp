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

/* ��ȡ������ɫ (��δ���㷨д�붥����ɫ) */
std::vector<QVector3D> MeshProcessor::extractColors() const {
    std::vector<QVector3D> cols; 
    cols.reserve(mesh.vertices.size());
    for (const auto& vp : mesh.vertices) {
        if (vp) {
            const auto& c = vp->color;
            cols.emplace_back((float)c.x(), (float)c.y(), (float)c.z());
        }
        else {
            cols.emplace_back(1.f, 1.f, 1.f);
        }
    }
    return cols;
}

//homework2
// �õ��Ƿ������
void MeshProcessor::processGeometry() {
    meanCurvature();
}
//ƽ������ʵ��
void MeshProcessor::meanCurvature() {
    int size = mesh.vertices.size();

    for (int i = 0; i < size; i++) {
        //����ÿ�����㣬��������һ�������Ӧ��ƽ������
        geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
        Eigen::Vector3d mean_curvature = { 0 , 0, 0 };
        Eigen::Vector3d total_value = { 0 , 0, 0 };//��¼�������ܺ�
        int count = 0;//��¼�������Ĵ�С
        do {
            total_value += hf->getEndVertex()->position;
            count++;
            hf = hf->pair->next;// ������һ������
        } while (hf != mesh.vertices[i]->halfEdge);
        mean_curvature = mesh.vertices[i]->position * count - total_value;// ��ɫ��Ӧ����ϵ��
        mesh.vertices[i]->color = mean_curvature * 255;
    }
}

void MeshProcessor::cotangentCurvature() {

}

void MeshProcessor::gaussianCurvature() {

}



