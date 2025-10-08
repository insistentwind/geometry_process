#include "mesh_processor.h"
#include <mesh_converter.h>
#include <iostream>

std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
MeshProcessor::processOBJData(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices) {
    
    // 步骤1：使用geometry模块的MeshConverter构建半边网格
    geometry::MeshConverter::buildMeshFromQtData(mesh, vertices, indices);

    // 步骤2：验证半边结构的正确性
    if (!mesh.isValid()) {
        std::cerr << "Warning: Generated half-edge mesh is invalid!" << std::endl;
    }

    // 步骤3：执行实际的几何处理操作（这是需要自己实现的部分）
    processGeometry();

    // 步骤4：使用geometry模块的MeshConverter将结果转回Qt格式
    return geometry::MeshConverter::convertMeshToQtData(mesh);
}

/* 提取顶点颜色 (若未来算法写入顶点颜色) */
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
// 用的是封闭曲面
void MeshProcessor::processGeometry() {
    //meanCurvature();
	cotangentCurvature();
}
//平均曲率实现
void MeshProcessor::meanCurvature() {
    int size = mesh.vertices.size();

    for (int i = 0; i < size; i++) {
        //对于每个顶点，计算它的一阶邻域对应的平均曲率
        geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
        Eigen::Vector3d mean_curvature = { 0 , 0, 0 };
        Eigen::Vector3d total_value = { 0 , 0, 0 };//记录定点数总和
        int count = 0;//记录这个邻域的大小
        do {
            total_value += hf->getEndVertex()->position;
            count++;
            hf = hf->pair->next;// 遍历下一个顶点
        } while (hf != mesh.vertices[i]->halfEdge);
        mean_curvature = mesh.vertices[i]->position * count - total_value;// 颜色对应基本系数
        mesh.vertices[i]->color = mean_curvature * 255;
    }
}
// cotangent曲率实现
void MeshProcessor::cotangentCurvature() {
	int size = mesh.vertices.size();

	//遍历每个顶点，先拿到一阶邻域的所有顶点个数
    for (int i = 0; i < size; i++) {
		if (mesh.vertices[i]->isBoundary()) continue;//跳过边界点
		double area = 0.0;//记录区域面积
        Eigen::Vector3d cotangent_curvature = { 0, 0, 0 };
		geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
        // 从这个点出发，先拿到所有的一阶邻域的顶点
		std::vector<geometry::Vertex*> one_ring_vertices;
        while(hf -> pair -> next != mesh.vertices[i]->halfEdge){
            one_ring_vertices.push_back(hf->getEndVertex());
            hf = hf->pair->next;
		}
		int one_ring_size = one_ring_vertices.size();

        for (int j = 0; j < one_ring_size; j++) {
			//开始计算这个顶点的cotangent曲率
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
            // 计算这两块三角形所占面积
			area += (v0v1.cross(v0vi)).norm() + (v2v1.cross(v2vi)).norm();
        }

		cotangent_curvature = cotangent_curvature / (2 * area);
		mesh.vertices[i]->color = cotangent_curvature * 255;
        std::cout << "vertex " << i << " cotangent_curvature: " << cotangent_curvature.transpose() << std::endl;
    }

}

void MeshProcessor::gaussianCurvature() {

}



