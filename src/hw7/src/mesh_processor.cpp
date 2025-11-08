#include "mesh_processor.h"
#include <mesh_converter.h>
#include <iostream>
#include <unordered_set>
#include <Eigen/sparse>

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
// 做tuttes emedding的重心坐标方法 harmonic coordinates
void MeshProcessor::processGeometry() {
	// 按照顺序排序,找出边界顶点，有序排列到圆盘上
	int vertexCount = mesh.getVertexCount();

	//geometry::HalfEdge* startHE = mesh.vertices[0]->halfEdge;//设置出发边是第一个顶点的半边

	// 关键问题是，如何从一个半边出发，有顺遍历整个网格边

	std::unordered_set<geometry::HalfEdge*> halfedgeVisitedSet;

	int boundary_index = 0;// 边界顶点索引

	int size = mesh.vertices.size();

	for (auto& he : mesh.halfEdges) {// 遍历所有半边
		if (he->pair != nullptr) continue;//如果不是边界半边，直接找下一个
		else if (halfedgeVisitedSet.find(he.get()) != halfedgeVisitedSet.end()) continue; //已经访问过了

		// 否则 就是没有访问过的边界半边
		geometry::HalfEdge* start = he.get();
		halfedgeVisitedSet.insert(start);// 标记为访问过
		start->vertex->boundary_index = boundary_index++;// 标记边界顶点的索引z

		geometry::HalfEdge* currentHE = start;// 以当前边界半边终点作为起点，找下一个边界半边
		// 这里要把和currentHE相连的所有边界半边都找出来
		do {
			geometry::HalfEdge* nextHE = currentHE->next;// 先找到当前边界半边的下一个半边
			while (nextHE && nextHE->pair != nullptr) {
				nextHE = nextHE->pair->next;// 查找一阶邻域是否有下一条边界半边
			}
			//if (nextHE == currentHE->next) break;
			// 为什么不需要判断 nextHE 是否为空？因为边界是封闭的，不会出现空的情况，至少存在一条边界边在当前这个顶点中是一阶邻域
			// 否则，是新的边界半边

			if (halfedgeVisitedSet.find(nextHE) == halfedgeVisitedSet.end())
				// 说明没有访问过
				nextHE->vertex->boundary_index = boundary_index++;// 标记边界顶点的索引


			halfedgeVisitedSet.insert(nextHE);// 标记为访问过
			currentHE = nextHE;// 更新当前边界半边
		} while (currentHE != start);
		// currentHE 必定会等于 start，因为一个边界环必定是闭合的！
	}

	Eigen::SparseMatrix<double> A(size, size);// 第一个参数是行数

	Eigen::VectorXd b_x = Eigen::VectorXd::Zero(size);

	Eigen::VectorXd b_y = Eigen::VectorXd::Zero(size);

	Eigen::VectorXd pos_x = Eigen::VectorXd::Zero(size);
	Eigen::VectorXd pos_y = Eigen::VectorXd::Zero(size);

	std::vector<Eigen::Triplet<double>> triplets;

	for (int i = 0; i < mesh.vertices.size(); i++) {
		mesh.vertices[i]->index = i;// 存储旧位置
	}

	// 下面开始处理曲面参数化
	for (int i = 0; i < mesh.vertices.size(); i++) {
		if (mesh.vertices[i]->boundary_index >= 0) {
			// 是边界顶点
			double theta = (double)2.0 * M_PI * mesh.vertices[i]->boundary_index / (double)boundary_index;
			double x = cos(theta);
			double y = sin(theta);
			//mesh.vertices[i]->position = Eigen::Vector3d(x, y, 0.0);

			triplets.emplace_back(i, i, 1.0);// 保持边界不动
			b_x[i] = x;
			b_y[i] = y;
		}
		else {
			// 计算调和重心坐标权值
			geometry::HalfEdge* he = mesh.vertices[i]->halfEdge;
			Eigen::Vector3d v0 = mesh.vertices[i]->position;
			double wi_sum = 0.0;
			// 先计算所有权值之和
			do {
				if (he->pair == nullptr) break;// 遇到边界半边，跳出
				Eigen::Vector3d vi = he->getEndVertex()->position;
				Eigen::Vector3d v_prev = he->pair->next->getEndVertex()->position;
				Eigen::Vector3d v_rear = he->next->getEndVertex()->position;

				double alpha_rear = acos((vi - v0).dot((v_rear - v0)) / ((vi - v0).norm() * (v_rear - v0).norm()));

				double alpha_prev = acos((v_prev - v0).dot(vi - v0) / ((v_prev - v0).norm() * (vi - v0).norm()));

				double length_ij = (vi - v0).norm();

				double wi = (tan(alpha_rear / 2) + tan(alpha_prev / 2)) / length_ij;

				wi_sum += wi;

				he = he->pair->next;// 遍历下一个顶点
			} while (he != mesh.vertices[i]->halfEdge);


			// 再次遍历，构建矩阵A

			do {
				if (he->pair == nullptr) break;// 遇到边界半边，跳出

				Eigen::Vector3d vi = he->getEndVertex()->position;
				Eigen::Vector3d v_prev = he->pair->next->getEndVertex()->position;
				Eigen::Vector3d v_rear = he->next->getEndVertex()->position;

				double alpha_rear = acos((vi - v0).dot((v_rear - v0)) / ((vi - v0).norm() * (v_rear - v0).norm()));

				double alpha_prev = acos((v_prev - v0).dot(vi - v0) / ((v_prev - v0).norm() * (vi - v0).norm()));

				double length_ij = (vi - v0).norm();

				double wi = (tan(alpha_rear / 2) + tan(alpha_prev / 2)) / length_ij;

				double fai_i = wi / wi_sum;

				triplets.emplace_back(i, he->getEndVertex()->index, -fai_i);// 邻接点权值为负
				triplets.emplace_back(i, i, fai_i);// 自身点权值为正

				he = he->pair->next;// 遍历下一个顶点
			} while (he != mesh.vertices[i]->halfEdge);
		}
	}
	// 开始构建稀疏矩阵
	A.setFromTriplets(triplets.begin(), triplets.end());
	// 求解线性方程组
	Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
	solver.compute(A);
	if (solver.info() != Eigen::Success) {
		std::cerr << "Decomposition failed!" << std::endl;
		return;
	}
	pos_x = solver.solve(b_x);
	pos_y = solver.solve(b_y);

	// 更新顶点位置
	for (int i = 0; i < size; i++) {
		mesh.vertices[i]->position.x() = pos_x[i];
		mesh.vertices[i]->position.y() = pos_y[i];
		mesh.vertices[i]->position.z() = 0.0;
	}

}