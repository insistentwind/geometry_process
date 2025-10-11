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
    //processGeometry();

	cotangentCurvature();

    // 步骤4：使用geometry模块的MeshConverter将结果转回Qt格式
    return geometry::MeshConverter::convertMeshToQtData(mesh);
}
// bilateral normal filtering + gauss-seidel position update
void MeshProcessor::processGeometry() {
	double sigma_r = 0.25;
	double sigma_s = 0.25;

	int flag = 1;
	int face_size = mesh.faces.size();
	// 迭代更新面法线
	double threshold = 1;
	do {
		threshold = 0;
		for (int i = 0; i < face_size; i++) {
			// 直接用面积大小来设置 sigma_s

			//face【i】的法向用双边滤波来拟合
			geometry::Face* f = mesh.faces[i].get();// 当前面为什么要用get？
			//f->computeNormal();
			Eigen::Vector3d new_normal = { 0 , 0, 0 };
			f->old_normal = f->normal;// 先存储旧法线

			Eigen::Vector3d fi = f->normal;
			Eigen::Vector3d point1 = f->halfEdge->vertex->position;
			Eigen::Vector3d point2 = f->halfEdge->next->vertex->position;
			Eigen::Vector3d point3 = f->halfEdge->next->next->vertex->position;
			Eigen::Vector3d center_i = (point1 + point2 + point3) / 3;

			f->center_point = center_i;// 面心存储起来

			geometry::HalfEdge* hf = f->halfEdge;
			double kp = 0.0;
			do {
				Eigen::Vector3d v0 = hf->vertex->position;
				Eigen::Vector3d v1 = hf->getEndVertex()->position;
				if (hf->pair == nullptr) {
					hf = hf->next;
					continue;// 跳过边界点
				}
				Eigen::Vector3d v2 = hf->pair->next->getEndVertex()->position;
				//拿到邻域三角形的三个顶点
				double Ws = 0.0;
				double Wr = 0.0;
				Eigen::Vector3d fj = (v1 - v0).cross(v2 - v0);
				double area = fj.norm() * 0.5;

				fj.normalize();

				Eigen::Vector3d center_j = (v0 + v1 + v2) / 3;

				Ws = exp(-std::abs((center_i - center_j).norm() * (center_i - center_j).norm()) / (2 * sigma_s * sigma_s));

				Wr = exp(-std::abs((1 - fi.dot(fj)) * (1 - fi.dot(fj))) / (2 * sigma_r * sigma_r));

				new_normal += fj * area * Ws * Wr;
				kp += area * Ws * Wr;

				hf = hf->next;
			} while (hf != f->halfEdge);

			new_normal /= kp;

			new_normal.normalize();
			f->normal = new_normal;// 新法线赋值
			threshold = std::min(threshold,1.0 - std::abs(f->normal.dot(f->old_normal)));
		}
		if(threshold < 1e-5) flag = 0;
	} while (flag != 0);
	//现在每个平面的法线设置好了，要更新每个顶点的位置
	int vector_size = mesh.vertices.size();

	do {
		

		for (int i = 0; i < vector_size; i++) {
			geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
			geometry::Vertex* vertex = hf->vertex;
			vertex->old_position = vertex->position;// 先存储旧位置
			Eigen::Vector3d gauss_seidel = { 0, 0, 0 };
			int count = 0;
			Eigen::Vector3d xi = vertex->old_position;

			do {
				geometry::Face* face = hf->face;
				Eigen::Vector3d cj = face->center_point;
				Eigen::Vector3d nj = face->normal;

				gauss_seidel += nj * nj.transpose() * (cj - xi);

				count++;
				if (hf->pair == nullptr) {
					break;
				}
				hf = hf->pair->next;
			} while (hf != mesh.vertices[i]->halfEdge);

			if (count > 0) {
				gauss_seidel /= count;
				vertex->position += gauss_seidel;
			}

			threshold = std::min(threshold, (vertex->position - vertex->old_position).norm());

		}


	} while (threshold > 1e-5);

}
// cotangent laplacian curvature
void MeshProcessor::cotangentCurvature() {
	int size = mesh.vertices.size();

	std::vector<double> curvature_magnitudes(size);
	double global_max_mag = -1e10;
	double global_min_mag = 1e10;
	double threshold = 1.0;

	do {
		//遍历每个顶点，先拿到一阶邻域的所有顶点个数
		for (int i = 0; i < size; i++) {
			mesh.vertices[i]->old_position = mesh.vertices[i]->position;
			if (mesh.vertices[i]->isBoundary()) continue;//跳过边界点
			double area = 0.0;//记录区域面积
			Eigen::Vector3d cotangent_curvature = { 0, 0, 0 };
			geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
			// 从这个点出发，先拿到所有的一阶邻域的顶点
			std::vector<geometry::Vertex*> one_ring_vertices;
			while (hf->pair->next != mesh.vertices[i]->halfEdge) {
				one_ring_vertices.push_back(hf->getEndVertex());
				hf = hf->pair->next;
			}
			int one_ring_size = one_ring_vertices.size();

			for (int j = 0; j < one_ring_size; j++) {
				//开始计算这个顶点的cotangent曲率
				geometry::Vertex* v0 = one_ring_vertices[(j - 1 + one_ring_size) % one_ring_size];
				geometry::Vertex* v1 = one_ring_vertices[j];
				geometry::Vertex* v2 = one_ring_vertices[(j + 1) % one_ring_size];
				geometry::Vertex* vi = mesh.vertices[i].get();

				Eigen::Vector3d v0v1 = v1->position - v0->position;
				Eigen::Vector3d v0vi = vi->position - v0->position;

				Eigen::Vector3d v2v1 = v1->position - v2->position;
				Eigen::Vector3d v2vi = vi->position - v2->position;

				//double cos_theta0 = v0v1.dot(v0vi) ;

				double cos_theta0 = v0v1.dot(v0vi) / (v0v1.norm() * v0vi.norm());

				//double cos_theta1 = (v2v1).dot(v2vi) ;
				double cos_theta1 = (v2v1).dot(v2vi) / (v2v1.norm() * v2vi.norm());
				cos_theta0 = std::min(cos_theta0, 0.9);
				cos_theta1 = std::min(cos_theta1, 0.9);
				double cot_0 = cos_theta0 / sqrt(1 - cos_theta0 * cos_theta0);

				double cot_1 = cos_theta1 / sqrt(1 - cos_theta1 * cos_theta1);

				cotangent_curvature += (cot_0 + cot_1) * (v1->position - vi->position);
				// 计算这两块三角形所占面积
				area += (v0v1.cross(v0vi)).norm() + (v2v1.cross(v2vi)).norm();//按照矩形来算了
			}

			cotangent_curvature = cotangent_curvature / (4 * area);
			// --- 最终归一化，计算模长，并记录 ---
			double curvature_magnitude = cotangent_curvature.norm();

			curvature_magnitudes[i] = curvature_magnitude;


			// 更新全局最大/最小值
			global_max_mag = std::max(global_max_mag, curvature_magnitude);
			global_min_mag = std::min(global_min_mag, curvature_magnitude);
			
			
			mesh.vertices[i]->position += cotangent_curvature * 0.0001;

			threshold = std::min(threshold, (mesh.vertices[i]->position - mesh.vertices[i]->old_position).norm());
		}
	} while (threshold > 1e-5);

}


	//int size = mesh.vertices.size();

 //   for (int i = 0; i < size; i++) {
	//	mesh.vertices[i]->old_normal = mesh.vertices[i]->normal;
 //       //对每个顶点都进行处理
 //       //double total_area = 0.0;// 总面积作为权值
	//	if (mesh.vertices[i]->isBoundary()) continue;//跳过边界点

 //       // 根据平面法向恢复顶点的法向
	//	/*mesh.vertices[i]->computeNormal();*/
	//	geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
	//	Eigen::Vector3d new_normal = { 0 , 0, 0 };
 //       do{
	//		Eigen::Vector3d v0 = hf->vertex->position;
 //           Eigen::Vector3d v1 = hf->getEndVertex()->position;
 //           hf = hf->pair->next;
	//		Eigen::Vector3d v2 = hf->getEndVertex()->position;
 //           //计算这个平面的面积和法向
	//		Eigen::Vector3d face_normal = (v1 - v0).cross(v2 - v0);
	//		double area = face_normal.norm() * 0.5f;
	//		new_normal += face_normal * area;
 //           //计算平面的face
 //       } while (hf != mesh.vertices[i]->halfEdge);
 //       new_normal.normalize();
	//	mesh.vertices[i]->normal = new_normal;// 新normal赋值
 //   }
 //   // 一层for循环结束，每个顶点都被赋予了新的法向

 //   // 下面开始恢复顶点位置
 //   for (int i = 0; i < size; i++) {

 //   }
