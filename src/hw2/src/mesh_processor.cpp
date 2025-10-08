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
    //meanCurvature();
	cotangentCurvature();
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
// cotangent����ʵ��
void MeshProcessor::cotangentCurvature() {
	int size = mesh.vertices.size();

	//����ÿ�����㣬���õ�һ����������ж������
    for (int i = 0; i < size; i++) {
		if (mesh.vertices[i]->isBoundary()) continue;//�����߽��
		double area = 0.0;//��¼�������
        Eigen::Vector3d cotangent_curvature = { 0, 0, 0 };
		geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
        // ���������������õ����е�һ������Ķ���
		std::vector<geometry::Vertex*> one_ring_vertices;
        while(hf -> pair -> next != mesh.vertices[i]->halfEdge){
            one_ring_vertices.push_back(hf->getEndVertex());
            hf = hf->pair->next;
		}
		int one_ring_size = one_ring_vertices.size();

        for (int j = 0; j < one_ring_size; j++) {
			//��ʼ������������cotangent����
			geometry::Vertex* v0 = one_ring_vertices[(j - 1 + one_ring_size) %one_ring_size];
			geometry::Vertex* v1 = one_ring_vertices[j];
			geometry::Vertex* v2 = one_ring_vertices[(j + 1)%one_ring_size];
			geometry::Vertex* vi = mesh.vertices[i].get();

			Eigen::Vector3d v0v1 = v1->position - v0->position;
			Eigen::Vector3d v0vi = vi->position - v0->position;

			Eigen::Vector3d v2v1 = v1->position - v2->position;
            Eigen::Vector3d v2vi = vi->position - v2->position;

			double cos_theta0 = v0v1.dot(v0vi) / (v0v1.norm() * v0vi.norm());

			double cos_theta1 = (v2v1).dot(v2vi) / (v2v1.norm() * v2vi.norm());

            cotangent_curvature += (cos_theta0 + cos_theta1) * (v1->position - vi->position);
            // ������������������ռ���
			area += (v0v1.cross(v0vi)).norm() + (v2v1.cross(v2vi)).norm();
        }

		cotangent_curvature = cotangent_curvature / (2 * area);
		mesh.vertices[i]->color = cotangent_curvature * 255;
        std::cout << "vertex " << i << " cotangent_curvature: " << cotangent_curvature.transpose() << std::endl;
    }

}

void MeshProcessor::gaussianCurvature() {

}



