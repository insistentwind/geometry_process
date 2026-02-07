#include "mesh_processor.h"
#include <mesh_converter.h>
#include <iostream>
#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/SVD>
#include <unordered_set>


std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
MeshProcessor::processOBJData(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices) {
    
    // 步骤1：使用geometry模块的MeshConverter构建半边网格
    geometry::MeshConverter::buildMeshFromQtData(mesh, vertices, indices);

    // 步骤2：验证半边结构的正确性
    if (!mesh.isValid()) {
        std::cerr << "Warning: Generated half-edge mesh is invalid!" << std::endl;
    }

	tuttes_embedding();

    // 步骤3：执行实际的几何处理操作（这是需要自己实现的部分）
    processGeometry();

    // 步骤4：使用geometry模块的MeshConverter将结果转回Qt格式
    return geometry::MeshConverter::convertMeshToQtData(mesh);
}

// local - global 框架 实现参数化映射
// Most-Isometric Parameterizations of Surface Meshes
void MeshProcessor::processGeometry() {
    // 把jacob矩阵作为优化对象, 这样可以略过系数k的优化
    // 意思是直接对jacob做等距变换？    
	int total_faces = static_cast<int>(mesh.faces.size());
	int nv = static_cast<int>(mesh.vertices.size());
	if (total_faces == 0 || nv == 0) return;

	// 参考实现的理论：先在 source(原始3D几何) 上预计算每个三角形的 area / xx / yy
	// 再用 target(当前2D参数化，来自 tutte) 计算每个面的 Jacobian，做极分解得到 R 与对称伸缩 S。
	// 然后只需要解一次线性系统得到新的参数化（这里等价取 t=1）。

	std::vector<Eigen::Vector3d> xx(total_faces);
	std::vector<Eigen::Vector3d> yy(total_faces);
	std::vector<double> area(total_faces, 0.0);
	std::vector<Eigen::Matrix2d> S(total_faces, Eigen::Matrix2d::Identity());
	std::vector<double> angle(total_faces, 0.0);

	std::vector<std::array<int, 3>> v_id(total_faces);

	for (int fi = 0; fi < total_faces; ++fi) {
		geometry::Face* face = mesh.faces[fi].get();
		geometry::HalfEdge* he = face->halfEdge;
		int i0 = he->vertex->index;
		int i1 = he->next->vertex->index;
		int i2 = he->next->next->vertex->index;
		v_id[fi] = { i0, i1, i2 };

		Eigen::Vector3d p0 = mesh.vertices[i0]->old_position;
		Eigen::Vector3d p1 = mesh.vertices[i1]->old_position;
		Eigen::Vector3d p2 = mesh.vertices[i2]->old_position;
		double a = (p1 - p0).cross(p2 - p0).norm() * 0.5;
		area[fi] = a;
		if (a < 1e-12) continue;

		xx[fi] = Eigen::Vector3d(p2.x() - p1.x(), p0.x() - p2.x(), p1.x() - p0.x());
		yy[fi] = Eigen::Vector3d(p1.y() - p2.y(), p2.y() - p0.y(), p0.y() - p1.y());

		Eigen::Vector3d ux(mesh.vertices[i0]->position.x(), mesh.vertices[i1]->position.x(), mesh.vertices[i2]->position.x());
		Eigen::Vector3d uy(mesh.vertices[i0]->position.y(), mesh.vertices[i1]->position.y(), mesh.vertices[i2]->position.y());

		Eigen::Matrix2d J;
		J << yy[fi].dot(ux) / (2.0 * a), xx[fi].dot(ux) / (2.0 * a),
			yy[fi].dot(uy) / (2.0 * a), xx[fi].dot(uy) / (2.0 * a);

		Eigen::JacobiSVD<Eigen::Matrix2d> svd(J, Eigen::ComputeFullU | Eigen::ComputeFullV);
		Eigen::Matrix2d V = svd.matrixV();
		Eigen::Matrix2d U = svd.matrixU();
		Eigen::Matrix2d R = U * V.transpose();
		if (R.determinant() < 0) {
			V.col(1) *= -1.0;
			R = U * V.transpose();
		}

		Eigen::Matrix2d Sigma = Eigen::Matrix2d::Zero();
		Sigma(0, 0) = svd.singularValues()(0);
		Sigma(1, 1) = svd.singularValues()(1);
		S[fi] = V * Sigma * V.transpose();
		angle[fi] = std::atan2(R(1, 0), R(1, 1));
	}

	// 固定最后一个点（参考实现是直接删变量，这里用 penalty 等价固定）
	int fixed = nv - 1;
	double u0 = mesh.vertices[fixed]->position.x();
	double v0 = mesh.vertices[fixed]->position.y();

	Eigen::SparseMatrix<double> A(2 * nv, 2 * nv);
	std::vector<Eigen::Triplet<double>> trip;
	trip.reserve(static_cast<size_t>(total_faces) * 3 * 3 * 4);

	for (int fi = 0; fi < total_faces; ++fi) {
		if (area[fi] < 1e-12) continue;
		double denom = area[fi] * area[fi] * 2.0;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				int vi = v_id[fi][i];
				int vj = v_id[fi][j];
				double w = (yy[fi](i) * yy[fi](j) + xx[fi](i) * xx[fi](j)) / denom;
				trip.emplace_back(2 * vi, 2 * vj, w);
				trip.emplace_back(2 * vi + 1, 2 * vj + 1, w);
			}
		}
	}

	const double penalty = 1e6;
	trip.emplace_back(2 * fixed, 2 * fixed, penalty);
	trip.emplace_back(2 * fixed + 1, 2 * fixed + 1, penalty);

	A.setFromTriplets(trip.begin(), trip.end());
	Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;
	solver.compute(A);
	if (solver.info() != Eigen::Success) {
		std::cout << "矩阵分解失败！" << std::endl;
		return;
	}

	// 取一次 t=1 的求解（等价化简你的作业：只要最终参数化）
	double t = 1.0;
	Eigen::Matrix2d I = Eigen::Matrix2d::Identity();
	Eigen::VectorXd b(2 * nv);
	b.setZero();
	b(2 * fixed) += penalty * u0;
	b(2 * fixed + 1) += penalty * v0;

	for (int fi = 0; fi < total_faces; ++fi) {
		if (area[fi] < 1e-12) continue;
		Eigen::Matrix2d R;
		double theta_t = angle[fi] * t;
		R << std::cos(theta_t), -std::sin(theta_t), std::sin(theta_t), std::cos(theta_t);
		Eigen::Matrix2d A_t = R * ((1.0 - t) * I + t * S[fi]);

		for (int i = 0; i < 3; i++) {
			int vi = v_id[fi][i];
			if (vi == fixed) continue;
			b(2 * vi) += yy[fi](i) * A_t(0, 0) / area[fi];
			b(2 * vi) += xx[fi](i) * A_t(0, 1) / area[fi];
			b(2 * vi + 1) += yy[fi](i) * A_t(1, 0) / area[fi];
			b(2 * vi + 1) += xx[fi](i) * A_t(1, 1) / area[fi];
		}
	}

	Eigen::VectorXd result = solver.solve(b);
	if (solver.info() != Eigen::Success) {
		std::cout << "求解失败！" << std::endl;
		return;
	}

	for (int i = 0; i < nv; ++i) {
		mesh.vertices[i]->position.x() = result(2 * i);
		mesh.vertices[i]->position.y() = result(2 * i + 1);
		mesh.vertices[i]->position.z() = 0.0;
	}
}



void MeshProcessor::tuttes_embedding() {

	int size = mesh.vertices.size();
	if (size == 0) return;
	for (auto& v : mesh.vertices) {
		v->boundary_index = -1; // 重置
	}

	// 遍历所有边界半边，标记边界顶点的 boundary_index
	// 按拓扑顺序遍历边界环** 很重要
	int boundary_size = 0;          // 总边界顶点数
	int boundary_cycle = 0;         // 边界环个数
	std::unordered_set<geometry::HalfEdge*> visitedBoundaryHE;// 访问过的边界半边
	// 遍历所有半边，找到边界半边
	for (auto& hePtr : mesh.halfEdges) {
		geometry::HalfEdge* start = hePtr.get();
		if (start->pair) continue;// 不是边界半边
		if (visitedBoundaryHE.count(start)) continue;// 如果遍历过了，就跳过

		geometry::HalfEdge* he = start;
		int step = 0;
		do {
			visitedBoundaryHE.insert(he);
			geometry::Vertex* v = he->vertex;
			if (v->boundary_index < 0) {
				v->boundary_index = boundary_size++; // 从 0 开始递增
			}

			// 跳到下一条边界半边
			geometry::HalfEdge* candidate = he->next;
			while (candidate && candidate->pair) {
				candidate = candidate->pair->next;
			}
			he = candidate;
			step++;
			if (step > (int)mesh.halfEdges.size()) { // 安全退出：异常拓扑
				std::cerr << "Warning: boundary walk aborted (non-manifold?)" << std::endl;
				break;
			}
		} while (he && he != start);

		boundary_cycle++;
	}
	// 记录每个边界环起点， 遍历渲染
	std::cout << "Boundary cycles: " << boundary_cycle
		<< " boundary vertices: " << boundary_size << std::endl;


	if (boundary_size < 3) {
		std::cerr << "Error: Boundary size is less than 3!" << std::endl;
		return;
	}
	Eigen::SparseMatrix<double> A(size, size);// 第一个参数是行数

	Eigen::VectorXd b_x = Eigen::VectorXd::Zero(size);

	Eigen::VectorXd b_y = Eigen::VectorXd::Zero(size);

	Eigen::VectorXd pos_x = Eigen::VectorXd::Zero(size);
	Eigen::VectorXd pos_y = Eigen::VectorXd::Zero(size);

	int column = 0;
	int count = 0;// 记录邻域大小
	for (int i = 0; i < size; i++) {
		mesh.vertices[i]->old_position = mesh.vertices[i]->position; // 记录旧位置

		if (mesh.vertices[i]->isBoundary()) {

			double t = (double)mesh.vertices[i]->boundary_index / (double)boundary_size;//横坐标设出来 [0, B-1], B=boundary_size
			double theta = 2.0 * M_PI * t;
			double x = std::cos(theta);
			double y = std::sin(theta);
			int index = mesh.vertices[i]->index;
			A.coeffRef(column, index) = 1;
			b_x(column) = x;
			b_y(column) = y;
			column++;
			continue;
		}
		// 内部的点
		else {
			geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
			count = 0;
			do {

				A.coeffRef(column, hf->pair->vertex->index) = -1;
				hf = hf->pair->next;// 遍历下一个顶点
				// i 行？
				count++;
			} while (hf != mesh.vertices[i]->halfEdge);
		}
		A.coeffRef(column, mesh.vertices[i].get()->index) = count;
		column++;
	}

	// 开始求解矩阵
	Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
	solver.compute(A);
	if (solver.info() != Eigen::Success) {
		std::cerr << "Decomposition failed!" << std::endl;
		return;
	}
	pos_x = solver.solve(b_x);
	pos_y = solver.solve(b_y);

	for (int i = 0; i < size; i++) {
		mesh.vertices[i]->position = { pos_x[i], pos_y[i], 0 };
	}


}