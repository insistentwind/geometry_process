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
	//cotangentCurvature();
	gaussianCurvature();
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
			area += (v0v1.cross(v0vi)).norm() + (v2v1.cross(v2vi)).norm();//���վ���������
        }

		cotangent_curvature = cotangent_curvature / (4 * area);
		mesh.vertices[i]->color = cotangent_curvature * 255;
        std::cout << "vertex " << i << " cotangent_curvature: " << cotangent_curvature.transpose() << std::endl;
    }

}

void MeshProcessor::gaussianCurvature() {
    int size = mesh.vertices.size();
    std::vector<double> curvature(size, 0);
    double max_curvature = 1;
    //����ÿ�����㣬���õ�һ����������ж������
    for (int i = 0; i < size; i++) {
        if (mesh.vertices[i]->isBoundary()) continue;//�����߽��
        
        double gauss_curvature = 0.0;

        double theta = 0.0;
		double area = 0.0;//��¼�������
        geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
        // ���������������õ����е�һ������Ķ���
        std::vector<geometry::Vertex*> one_ring_vertices;
        while (hf->pair->next != mesh.vertices[i]->halfEdge) {
            one_ring_vertices.push_back(hf->getEndVertex());
            hf = hf->pair->next;
        }
        int one_ring_size = one_ring_vertices.size();

        for (int j = 0; j < one_ring_size; j++) {
            //��ʼ������������gauss����
            geometry::Vertex* v0 = one_ring_vertices[(j - 1 + one_ring_size) % one_ring_size];
            geometry::Vertex* v1 = one_ring_vertices[j];
            geometry::Vertex* vi = mesh.vertices[i].get();

			Eigen::Vector3d viv1 = v1->position - vi->position;
			Eigen::Vector3d viv0 = v0->position - vi->position;

			theta += acos(viv1.dot(viv0) / (viv1.norm() * viv0.norm()));
            //theta += acos(viv1.dot(viv0));

			area += (viv1.cross(viv0)).norm() / 2.0;
			
        }
        gauss_curvature = (2 * M_PI - theta) / area;

        curvature[i] = gauss_curvature;
        max_curvature = std::min(max_curvature, std::abs(gauss_curvature));
		std::cout << "vertex " << i << " gauss_curvature: " << gauss_curvature << std::endl;
		//mesh.vertices[i]->color = Eigen::Vector3d(gauss_curvature, gauss_curvature, gauss_curvature) * 255;
    }
    std::cout << "max gauss_curvature: " << max_curvature << std::endl;

    // ���Ʋ��������ڲ��Ұٷ�λ��
    std::vector<double> sorted_curvatures = curvature;
    std::sort(sorted_curvatures.begin(), sorted_curvatures.end());

    // --- 1. ȷ��³����һ����Χ ---
    // ʹ�� 95% �� 5% �ٷ�λ�����ų���˵� 10% �쳣ֵ
    int idx_95 = (int)(size * 0.95);
    int idx_05 = (int)(size * 0.05);

    double K_robust_max = sorted_curvatures[idx_95];
    double K_robust_min = sorted_curvatures[idx_05];

    // ³����Χ�ĳ���
    double K_RANGE = K_robust_max - K_robust_min;

    // --- 2. ��ɫӳ��ѭ�� ---
    if (K_RANGE < 1e-9) {
        // �����������ʶ���ͬ����ӽ�����������������
        K_RANGE = 1.0;
        K_robust_min = K_robust_max - 1.0;
    }

    for (int i = 0; i < size; i++) {
        // if (mesh.vertices[i]->isBoundary()) continue; // �����������������߽��

        double K_i = curvature[i];

        // a) �ü�/ǯ�� K_i ��³����Χ [K_robust_min, K_robust_max]
        K_i = std::min(K_i, K_robust_max);
        K_i = std::max(K_i, K_robust_min);

        // b) ��һ���� [0, 1] ��Χ (K_norm = 0 �� K_robust_min, K_norm = 1 �� K_robust_max)
        double K_norm = (K_i - K_robust_min) / K_RANGE;

        // c) ����ʽ�߶Աȶ�ɫ�� (�� -> �� -> ��)
        Eigen::Vector3d color;

        // K_norm < 0.5: ������ (0 -> 1)
        if (K_norm < 0.5) {
            double t = K_norm * 2.0; // ��Χ [0, 1]
            color[0] = 0.0;          // ��ɫ����: 0
            color[1] = t;            // ��ɫ����: 0 -> 1
            color[2] = 1.0 - t;      // ��ɫ����: 1 -> 0
        }
        // K_norm >= 0.5: �̵��� (1 -> 0)
        else {
            double t = (K_norm - 0.5) * 2.0; // ��Χ [0, 1]
            color[0] = t;            // ��ɫ����: 0 -> 1
            color[1] = 1.0 - t;      // ��ɫ����: 1 -> 0
            color[2] = 0.0;          // ��ɫ����: 0
        }

        // ת��Ϊ [0, 255] ��Χ
        mesh.vertices[i]->color = color * 255.0;
    }


}



