#include "mesh_processor.h"
#include <mesh_converter.h>
#include <iostream>
#include <Eigen/Dense>
#include <algorithm>
#include <Eigen/Sparse>

std::pair<std::vector<QVector3D>, std::vector<unsigned int>>
MeshProcessor::processOBJData(const std::vector<QVector3D>& vertices,
	const std::vector<unsigned int>& indices) {

	// 步骤1：使用geometry模块的MeshConverter构建半边网格
	geometry::MeshConverter::buildMeshFromQtData(mesh, vertices, indices);

	// 步骤2：验证半边结构的正确性
	if (!mesh.isValid()) {
		std::cerr << "Warning: Generated half-edge mesh is invalid!" << std::endl;
	}

	for (auto& vertex : mesh.vertices) {
		vertex->old_position = vertex->position;
	}
	// 在第一步arap之前，先把老顶点保存起来

	//// 步骤3：执行实际的几何处理操作（这是需要自己实现的部分）
	//processGeometry();

	// 步骤4：使用geometry模块的MeshConverter将结果转回Qt格式
	return geometry::MeshConverter::convertMeshToQtData(mesh);
}

// arap变形
void MeshProcessor::processGeometry() {
	int v_size = mesh.vertices.size();

	// ===== 调试输出 =====
	int fixed_count = 0;
	for (const auto& v : mesh.vertices) {
		if (v->fixed) fixed_count++;
	}
	std::cout << "[ARAP] Fixed vertices: " << fixed_count << " / " << v_size << std::endl;
	
	if (fixed_count == 0) {
		std::cout << "[ARAP] ERROR: No fixed vertices! Aborting." << std::endl;
		return;
	}

	// ===== Local 步骤：计算每个顶点的旋转矩阵 =====
	std::vector<Eigen::Matrix3d> rotations(v_size);
	
	for (int i = 0; i < v_size; i++) {
		if (mesh.vertices[i]->fixed) {
			// 固定点不需要计算旋转矩阵，使用单位矩阵
			rotations[i] = Eigen::Matrix3d::Identity();
			continue;
		}
		
		// 非固定点：计算协方差矩阵 J
		Eigen::Matrix3d J = Eigen::Matrix3d::Zero();
		geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
		Eigen::Vector3d pi_new = mesh.vertices[i]->position;
		Eigen::Vector3d pi_old = mesh.vertices[i]->old_position;
		
		do {
			Eigen::Vector3d pj_new = hf->next->vertex->position;
			Eigen::Vector3d pj_old = hf->next->vertex->old_position;
			
			// 计算 cotangent 权重
			double wij = wij_caculate(hf, i);
			
			// 构建协方差矩阵：J = sum(wij * (pi_old - pj_old) * (pi_new - pj_new)^T)
			Eigen::Vector3d eij_old = pi_old - pj_old;
			Eigen::Vector3d eij_new = pi_new - pj_new;
			J += wij * eij_old * eij_new.transpose();
			
			hf = hf->pair->next;
		} while (hf != mesh.vertices[i]->halfEdge);
		
		// SVD 分解求旋转矩阵
		Eigen::JacobiSVD<Eigen::Matrix3d> svd(J, Eigen::ComputeFullU | Eigen::ComputeFullV);
		Eigen::Matrix3d U = svd.matrixU();
		Eigen::Matrix3d V = svd.matrixV();
		
		// 确保是旋转矩阵（行列式为正）
		Eigen::Matrix3d R = V * U.transpose();
		if (R.determinant() < 0) {
			V.col(2) *= -1;
			R = V * U.transpose();
		}
		
		rotations[i] = R;
	}

	// ===== Global 步骤：构建并求解线性系统 Ax = b =====
	std::vector<Eigen::Triplet<double>> triplets;
	Eigen::MatrixXd B = Eigen::MatrixXd::Zero(v_size, 3);
	
	for (int i = 0; i < v_size; i++) {
		if (mesh.vertices[i]->fixed) {
			// ===== Fixed 点：使用恒等约束 =====
			triplets.push_back(Eigen::Triplet<double>(i, i, 1.0));
			
			B(i, 0) = mesh.vertices[i]->position.x();
			B(i, 1) = mesh.vertices[i]->position.y();
			B(i, 2) = mesh.vertices[i]->position.z();
		}
		else {
			// ===== 非 Fixed 点：使用 ARAP 能量最小化方程 =====
			geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
			Eigen::Vector3d pi_old = mesh.vertices[i]->old_position;
			Eigen::Vector3d rhs = Eigen::Vector3d::Zero();
			
			double wii_sum = 0.0;
			
			do {
				int j = hf->next->vertex->index;
				Eigen::Vector3d pj_old = hf->next->vertex->old_position;
				
				double wij = wij_caculate(hf, i);
				
				// 矩阵 A 的构建
				wii_sum += wij;
				triplets.push_back(Eigen::Triplet<double>(i, j, -wij));
				
				// 右端项 b
				rhs += wij * 0.5 * (rotations[i] + rotations[j]) * (pi_old - pj_old);
				
				hf = hf->pair->next;
			} while (hf != mesh.vertices[i]->halfEdge);
			
			triplets.push_back(Eigen::Triplet<double>(i, i, wii_sum));
			
			B(i, 0) = rhs.x();
			B(i, 1) = rhs.y();
			B(i, 2) = rhs.z();
		}
	}
	
	// 构建稀疏矩阵
	Eigen::SparseMatrix<double> A(v_size, v_size);
	A.setFromTriplets(triplets.begin(), triplets.end());
	
	// ===== 求解线性系统 =====
	Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
	solver.compute(A);
	
	if (solver.info() != Eigen::Success) {
		std::cerr << "[ARAP] ERROR: Matrix factorization FAILED!" << std::endl;
		return;
	}
	std::cout << "[ARAP] Matrix factorization successful" << std::endl;
	
	Eigen::MatrixXd new_pos = solver.solve(B);
	
	if (solver.info() != Eigen::Success) {
		std::cerr << "[ARAP] ERROR: Linear system solve FAILED!" << std::endl;
		return;
	}
	std::cout << "[ARAP] Linear system solved successfully" << std::endl;
	
	// ===== 更新顶点位置（只更新非 fixed 点）=====
	for (int i = 0; i < v_size; i++) {
		if (!mesh.vertices[i]->fixed) {
			mesh.vertices[i]->position = Eigen::Vector3d(new_pos(i, 0), new_pos(i, 1), new_pos(i, 2));
		} else {
			// 验证 fixed 点是否被正确约束
			Eigen::Vector3d expected = mesh.vertices[i]->position;
			Eigen::Vector3d solved(new_pos(i, 0), new_pos(i, 1), new_pos(i, 2));
			double error = (expected - solved).norm();
			if (error > 1e-6) {
				std::cout << "[ARAP] WARNING: Fixed vertex " << i 
				       << " error = " << error << std::endl;
			}
		}
	}
}

// 计算wij
double MeshProcessor::wij_caculate(geometry::HalfEdge* hf, int i) {
	Eigen::Vector3d pi_old = mesh.vertices[i]->old_position;
	Eigen::Vector3d pj_old = hf->next->vertex->old_position;
	Eigen::Vector3d p1_old = hf->next->getEndVertex()->old_position;
	Eigen::Vector3d p0_old = hf->pair->next->getEndVertex()->old_position;

	Eigen::Vector3d e1i_old = pi_old - p1_old;
	Eigen::Vector3d e1j_old = pj_old - p1_old;

	Eigen::Vector3d e0i_old = pi_old - p0_old;
	Eigen::Vector3d e0j_old = pj_old - p0_old;

	double cot_alpha = e1i_old.dot(e1j_old) / e1i_old.cross(e1j_old).norm();
	double cot_beta = e0i_old.dot(e0j_old) / e0i_old.cross(e0j_old).norm();

	const double max_cot_abs = 10; // 设置一个合理的绝对值上限, e.g., 100,000 保证是正值
	cot_alpha = std::abs(std::max(- max_cot_abs, std::min(max_cot_abs, cot_alpha)));
	cot_beta = std::abs(std::max(-max_cot_abs, std::min(max_cot_abs, cot_beta)));

	return (cot_alpha + cot_beta) / 2.0;
}

// ================== ARAP交互API方法实现 ==================

void MeshProcessor::beginArapSession() {
	// 保存当前所有顶点位置为old_position（变形前的参考位置）
	for (auto& vertex : mesh.vertices) {
		vertex->old_position = vertex->position;
		vertex->fixed = false;
		vertex->handle = false;  // 清空handle标记
	}
	std::cout << "[MeshProcessor] ARAP session started, saved " << mesh.vertices.size()
		<< " vertex positions as old_position" << std::endl;
}

void MeshProcessor::endArapSession() {
	// 清空所有 ARAP 相关标记
	for (auto& vertex : mesh.vertices) {
		vertex->fixed = false;
		vertex->handle = false;
	}
	std::cout << "[MeshProcessor] ARAP session ended" << std::endl;
}

void MeshProcessor::setFixedVertex(int index, bool fixed) {
	if (index < 0 || index >= static_cast<int>(mesh.vertices.size())) {
		std::cerr << "[MeshProcessor] Invalid vertex index: " << index << std::endl;
		return;
	}
	mesh.vertices[index]->fixed = fixed;
	std::cout << "[MeshProcessor] Vertex " << index << " set to "
		<< (fixed ? "FIXED" : "FREE") << std::endl;
}

void MeshProcessor::setFixedVertices(const std::vector<int>& indices) {
	for (int index : indices) {
		setFixedVertex(index, true);
	}
}

void MeshProcessor::setHandleVertex(int index, bool handle) {
	if (index < 0 || index >= static_cast<int>(mesh.vertices.size())) {
		std::cerr << "[MeshProcessor] Invalid vertex index: " << index << std::endl;
		return;
	}
	
	// 同一时间只能有一个 handle 点，先清除旧的
	if (handle) {
		clearHandleVertex();
	}
	
	mesh.vertices[index]->handle = handle;
	std::cout << "[MeshProcessor] Vertex " << index << " set to "
		<< (handle ? "HANDLE" : "NOT_HANDLE") << std::endl;
}

void MeshProcessor::clearFixedVertices() {
	for (auto& vertex : mesh.vertices) {
		vertex->fixed = false;
	}
	std::cout << "[MeshProcessor] Cleared all fixed vertices" << std::endl;
}

void MeshProcessor::clearHandleVertex() {
	for (auto& vertex : mesh.vertices) {
		vertex->handle = false;
	}
	std::cout << "[MeshProcessor] Cleared handle vertex" << std::endl;
}

int MeshProcessor::getHandleIndex() const {
	for (size_t i = 0; i < mesh.vertices.size(); ++i) {
		if (mesh.vertices[i]->handle) {
			return static_cast<int>(i);
		}
	}
	return -1;  // 没有 handle 点
}

std::pair<std::vector<QVector3D>, std::vector<unsigned int>>
MeshProcessor::applyArapDrag(int handleIndex, const QVector3D& newPosition) {
	// 验证 handleIndex 有效性
	if (handleIndex < 0 || handleIndex >= static_cast<int>(mesh.vertices.size())) {
		std::cerr << "[MeshProcessor] Invalid handle vertex index: " << handleIndex << std::endl;
		return geometry::MeshConverter::convertMeshToQtData(mesh);
	}

	std::cout << "[MeshProcessor] Dragging vertex " << handleIndex 
	          << " to (" << newPosition.x() << ", " << newPosition.y() << ", " << newPosition.z() << ")" << std::endl;

	// ===== 核心逻辑 =====
	// 1. 更新当前拖动点（handle点）的位置为完整 3D 坐标
	double original_z = mesh.vertices[handleIndex]->position.z();
	mesh.vertices[handleIndex]->position = Eigen::Vector3d(
		newPosition.x(), 
		newPosition.y(), 
		newPosition.z()  // 使用完整 3D 坐标
		//original_z
	);

	// 2. 临时将handle点标记为fixed（作为ARAP约束）
	bool wasFixed = mesh.vertices[handleIndex]->fixed;
	mesh.vertices[handleIndex]->fixed = true;

	// 3. 执行 ARAP 优化（fixed点和handle点都保持不动，其他点通过ARAP计算）
	processGeometry();

	// 4. 恢复handle点的fixed状态（如果原本不是fixed点）
	if (!wasFixed) {
		mesh.vertices[handleIndex]->fixed = false;
	}
	// ===================

	// 返回变形后的mesh
	return geometry::MeshConverter::convertMeshToQtData(mesh);
}