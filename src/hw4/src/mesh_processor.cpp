#include "mesh_processor.h"
#include <mesh_converter.h>
#include <iostream>
#include <Eigen/Sparse>
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

	// 步骤3：执行实际的几何处理操作（这是需要自己实现的部分）
	//processGeometry();
    //processGeometry_ultimate();
    LSCM();

	// 步骤4：使用geometry模块的MeshConverter将结果转回Qt格式
	return geometry::MeshConverter::convertMeshToQtData(mesh);
}

// Tutte's embedding parameterization
void MeshProcessor::processGeometry() {
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


	if(boundary_size < 3) {
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
		if (mesh.vertices[i]->isBoundary()) {

			//double x = (double)mesh.vertices[i]->boundary_index / (double)boundary_size * 2.0;//横坐标设出来
			//double y = 0;

			//if (x < 1) {
			//	y = 1 - sqrt(1 - (x - 1) * (x - 1));
			//}
			//else
			//	y = y = 1 + sqrt(1 - (x - 1) * (x - 1));
			//int index = mesh.vertices[i]->index;
			//// t 在 0 - 2pi上, 圆的边界是0 - 2pi
			//A.coeffRef(column, index) = 1;
			//b_x(column) = x;// 这是 x 坐标
			//b_y(column) = y;// 这是 y 坐标

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


void MeshProcessor::processGeometry_ultimate() {
    int n = (int)mesh.vertices.size();
    if (n == 0) return;

    // 重置 boundary_index
    for (auto& v : mesh.vertices) v->boundary_index = -1;

    // 收集所有边界环
    std::vector<std::vector<geometry::HalfEdge*>> loops;
    std::unordered_set<geometry::HalfEdge*> visitedHE;

    for (auto& hePtr : mesh.halfEdges) {
        geometry::HalfEdge* start = hePtr.get();
        if (start->pair) continue;                // 只从边界半边开始
        if (visitedHE.count(start)) continue;

        std::vector<geometry::HalfEdge*> loop;
        geometry::HalfEdge* he = start;
        int guard = 0;
        do {
            loop.push_back(he);// 直接推送半边，起点为当前点
            visitedHE.insert(he);
            // 找下一条边界半边
            geometry::HalfEdge* cand = he->next;
            while (cand && cand->pair) {
                cand = cand->pair->next;
            }
            he = cand;
            if (++guard > (int)mesh.halfEdges.size()) {
                std::cerr << "Warning: boundary traversal aborted\n";
                break;
            }
        } while (he && he != start);

        if (!loop.empty()) loops.push_back(loop);
    }

    if (loops.empty()) {
        std::cerr << "No boundary loops found.\n";
        return;
    }

    // 给所有边界顶点分配连续的 global boundary_index，并记录每环局部顺序
    int globalBoundaryCount = 0;
    struct LoopInfo {
        std::vector<geometry::Vertex*> verts;
    };
    std::vector<LoopInfo> loopInfos;
    loopInfos.reserve(loops.size());

    for (size_t li = 0; li < loops.size(); ++li) {
        LoopInfo info;
        auto& loopHEs = loops[li];
        // 按半边起点顶点顺序
        for (auto* he : loopHEs) {
            geometry::Vertex* v = he->vertex;
            if (v->boundary_index < 0) {
                v->boundary_index = globalBoundaryCount++;
                info.verts.push_back(v);
            }
        }
        loopInfos.push_back(std::move(info));
    }

    if (globalBoundaryCount < 3) {
        std::cerr << "Boundary vertex count < 3.\n";
        return;
    }

    // 预留稀疏矩阵
    Eigen::SparseMatrix<double> A(n, n);
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(n * 6);

    Eigen::VectorXd bx = Eigen::VectorXd::Zero(n);
    Eigen::VectorXd by = Eigen::VectorXd::Zero(n);

    // 多环：同心圆嵌入（半径递减）
    // 外环半径 1.0，后续 r = 1.0 - 0.3 * 

    int up = 2;
    for (size_t li = 0; li < loopInfos.size(); ++li) {
       /* double r = 1.0 - 0.3 * li;*/
        double r = 1.0; // 圆的半径
        double a = up * li;// 参数方程常量系数
        if (r <= 0.05) r = 0.05; // 防止太小
        auto& verts = loopInfos[li].verts;
        int B = (int)verts.size();
        for (int k = 0; k < B; ++k) {
            geometry::Vertex* v = verts[k];
            double t = double(k) / double(B); // 0..(B-1)/B
            double theta = 2.0 * M_PI * t;
            double x = r * std::cos(theta) + a;
            double y = r * std::sin(theta);
            triplets.emplace_back(v->index, v->index, 1.0);
            bx(v->index) = x;
            by(v->index) = y;
        }
    }

    // 内部顶点：Uniform Laplacian
    for (auto& vPtr : mesh.vertices) {
        geometry::Vertex* v = vPtr.get();
        if (v->boundary_index >= 0) continue;

        geometry::HalfEdge* startHE = v->halfEdge;
        if (!startHE) {
            triplets.emplace_back(v->index, v->index, 1.0);
            bx(v->index) = v->position.x();
            by(v->index) = v->position.y();
            continue;
        }

        int degree = 0;
        geometry::HalfEdge* he = startHE;
        int guard = 0;
        do {
            if (!he->pair) { // 实际是边界顶点，回退锚定
                degree = -1;
                break;
            }
            geometry::Vertex* nbr = he->next->vertex;
            if (nbr) {
                triplets.emplace_back(v->index, nbr->index, -1.0);
                degree++;
            }
            he = he->pair->next;
            if (++guard > v->getDegree() + 5) break;
        } while (he && he != startHE);

        if (degree > 0) {
            triplets.emplace_back(v->index, v->index, double(degree));
        }
        else {
            triplets.emplace_back(v->index, v->index, 1.0);
            bx(v->index) = v->position.x();
            by(v->index) = v->position.y();
        }
    }

    A.setFromTriplets(triplets.begin(), triplets.end());

    Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
    solver.compute(A);
    if (solver.info() != Eigen::Success) {
        std::cerr << "Decomposition failed.\n";
        return;
    }
    Eigen::VectorXd solx = solver.solve(bx);
    Eigen::VectorXd soly = solver.solve(by);
    if (solver.info() != Eigen::Success) {
        std::cerr << "Solve failed.\n";
        return;
    }

    for (auto& vPtr : mesh.vertices) {
        geometry::Vertex* v = vPtr.get();
        v->position = Eigen::Vector3d(solx[v->index], soly[v->index], 0.0);
    }
}


void MeshProcessor::LSCM() {
    // 修正版 LSCM: 避免退化成一条直线
    // 退化常见原因：所有三角形展开后 y 分量接近 0 (长条/共线)，或约束只在一条轴上。
    // 稳定策略：
    // 1. 精确按公式构造 z_j, z_k 展开 (i 为原点, j 在 x 轴)。
    // 2. 若 y_k 非正或过小，用法线方向生成一个最小正值(保持共形结构)。
    // 3. 两个锚点放在 (0,0) 与 (1,0)。不做额外缩放，避免整体压缩。
    // 4. 面方程使用单位权或面积权 (这里用面积权提升稳健性)。

    int n = (int)mesh.vertices.size();
    int f = (int)mesh.faces.size();
    if (n == 0 || f == 0) return;

    // 记录原始 3D 到 old_position (若之前未写过)。
    for (auto& v : mesh.vertices) v->old_position = v->position;

    // 选两个边界锚点: 最远距离 (若边界不足用任意两点)
    int anchorA = -1, anchorB = -1;
    for (int i = 0; i < n; ++i) if (mesh.vertices[i]->isBoundary()) { anchorA = i; break; }
    if (anchorA >= 0) {
        double maxD = -1.0; Eigen::Vector3d pA = mesh.vertices[anchorA]->old_position;
        for (int i = 0; i < n; ++i) if (i != anchorA && mesh.vertices[i]->isBoundary()) {
            double d = (mesh.vertices[i]->old_position - pA).norm(); if (d > maxD) { maxD = d; anchorB = i; }
        }
    }
    if (anchorA < 0 || anchorB < 0) { // 无足够边界
        anchorA = 0; anchorB = (n > 1 ? 1 : 0); double maxD = -1.0;
        for (int i = 0; i < n; i++)for (int j = i + 1; j < n; j++) { double d = (mesh.vertices[i]->old_position - mesh.vertices[j]->old_position).norm(); if (d > maxD) { maxD = d; anchorA = i; anchorB = j; } }
    }
    if (anchorA == anchorB) return;

    // 系统：2*f + 4 行，2*n 列 (u(0..n-1), v(0..n-1))
    int rows = 2 * f + 4;
    int cols = 2 * n;
    std::vector<Eigen::Triplet<double>> triplets;
    Eigen::VectorXd b = Eigen::VectorXd::Zero(rows);

    auto uId = [&](int vi) {return vi; };
    auto vId = [&](int vi) {return vi + n; };

    int r = 0;
    for (int fi = 0; fi < f; ++fi) {
        auto face = mesh.faces[fi].get();
        auto he0 = face->halfEdge; auto he1 = he0->next; auto he2 = he1->next;
        int i = he0->vertex->index;
        int j = he1->vertex->index;
        int k = he2->vertex->index;

        Eigen::Vector3d p_i = mesh.vertices[i]->old_position;
        Eigen::Vector3d p_j = mesh.vertices[j]->old_position;
        Eigen::Vector3d p_k = mesh.vertices[k]->old_position;
        Eigen::Vector3d e_ij = p_j - p_i;
        Eigen::Vector3d e_ik = p_k - p_i;
        double Lij = e_ij.norm();
        double Lik = e_ik.norm();
        if (Lij < 1e-14 || Lik < 1e-14) continue; // 退化跳过

        // 展开: j 在 (Lij,0)
        double xk = e_ik.dot(e_ij) / Lij; // k 在 e_ij 方向投影
        double yk2 = std::max(0.0, e_ik.squaredNorm() - xk * xk);
        double yk = (yk2 <= 1e-20 ? 1e-10 : std::sqrt(yk2)); // 极小则给一个正值防止整片拉成直线

        // r = (z_j - z_i)/(z_k - z_i) = Lij / (xk + i yk)
        std::complex<double> zj(Lij, 0.0); std::complex<double> zk(xk, yk);
        if (std::abs(zk) < 1e-14) continue; // 退化
        std::complex<double> r_c = zj / zk;
        double rRe = r_c.real();
        double rIm = r_c.imag();

        // 面面积权重 (用原 3D 面积)
        double area2 = e_ij.cross(e_ik).norm();
        double w = 0.5 * area2;
        // 约束1: (u_j-u_i) - rRe*(u_k-u_i) + rIm*(v_k-v_i) = 0
        triplets.emplace_back(r, uId(j), w * 1.0);
        triplets.emplace_back(r, uId(i), w * -1.0);
        triplets.emplace_back(r, uId(k), w * -rRe);
        triplets.emplace_back(r, uId(i), w * rRe); // 加回 rRe*u_i
        triplets.emplace_back(r, vId(k), w * rIm);
        triplets.emplace_back(r, vId(i), w * -rIm);
        r++;
        // 约束2: (v_j-v_i) - rRe*(v_k-v_i) - rIm*(u_k-u_i) = 0
        triplets.emplace_back(r, vId(j), w * 1.0);
        triplets.emplace_back(r, vId(i), w * -1.0);
        triplets.emplace_back(r, vId(k), w * -rRe);
        triplets.emplace_back(r, vId(i), w * rRe);
        triplets.emplace_back(r, uId(k), w * -rIm);
        triplets.emplace_back(r, uId(i), w * rIm);
        r++;
    }

    // 锚点约束: uA=0, vA=0, uB=1, vB=0
    triplets.emplace_back(r, uId(anchorA), 1.0); b[r] = 0.0; r++;
    triplets.emplace_back(r, vId(anchorA), 1.0); b[r] = 0.0; r++;
    triplets.emplace_back(r, uId(anchorB), 1.0); b[r] = 1.0; r++;
    triplets.emplace_back(r, vId(anchorB), 1.0); b[r] = 0.0; r++;

    int finalRows = r;
    Eigen::SparseMatrix<double> M(finalRows, cols);
    M.setFromTriplets(triplets.begin(), triplets.end());
    Eigen::VectorXd b_use = b.head(finalRows);

    // 最小二乘: (M^T M) x = M^T b
    Eigen::SparseMatrix<double> MtM = M.transpose() * M;
    Eigen::VectorXd MtB = M.transpose() * b_use;
    for (int i = 0; i < cols; i++) MtM.coeffRef(i, i) += 1e-10; // 正则

    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver; solver.compute(MtM);
    if (solver.info() != Eigen::Success) { std::cerr << "LSCM: factorization failed" << std::endl; return; }
    Eigen::VectorXd sol = solver.solve(MtB);
    if (solver.info() != Eigen::Success) { std::cerr << "LSCM: solve failed" << std::endl; return; }

    for (int i = 0; i < n; ++i) { double u = sol[uId(i)], v = sol[vId(i)]; mesh.vertices[i]->position = { (float)u,(float)v,0.0f }; }
}