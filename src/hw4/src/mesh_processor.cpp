#include "mesh_processor.h"
#include <mesh_converter.h>
#include <iostream>
#include <Eigen/Sparse>
#include <unordered_set>

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
	//processGeometry();
    //processGeometry_ultimate();
    LSCM();

	// ����4��ʹ��geometryģ���MeshConverter�����ת��Qt��ʽ
	return geometry::MeshConverter::convertMeshToQtData(mesh);
}

// Tutte's embedding parameterization
void MeshProcessor::processGeometry() {
	int size = mesh.vertices.size();
	if (size == 0) return;
	for (auto& v : mesh.vertices) {
		v->boundary_index = -1; // ����
	}

	// �������б߽��ߣ���Ǳ߽綥��� boundary_index
	// ������˳������߽绷** ����Ҫ
	int boundary_size = 0;          // �ܱ߽綥����
	int boundary_cycle = 0;         // �߽绷����
	std::unordered_set<geometry::HalfEdge*> visitedBoundaryHE;// ���ʹ��ı߽���
	// �������а�ߣ��ҵ��߽���
	for (auto& hePtr : mesh.halfEdges) {
		geometry::HalfEdge* start = hePtr.get();
		if (start->pair) continue;// ���Ǳ߽���
		if (visitedBoundaryHE.count(start)) continue;// ����������ˣ�������

		geometry::HalfEdge* he = start;
		int step = 0;
		do {
			visitedBoundaryHE.insert(he);
			geometry::Vertex* v = he->vertex;
			if (v->boundary_index < 0) {
				v->boundary_index = boundary_size++; // �� 0 ��ʼ����
			}

			// ������һ���߽���
			geometry::HalfEdge* candidate = he->next;
			while (candidate && candidate->pair) {
				candidate = candidate->pair->next;
			}
			he = candidate;
			step++;
			if (step > (int)mesh.halfEdges.size()) { // ��ȫ�˳����쳣����
				std::cerr << "Warning: boundary walk aborted (non-manifold?)" << std::endl;
				break;
			}
		} while (he && he != start);

		boundary_cycle++;
	}
    // ��¼ÿ���߽绷��㣬 ������Ⱦ




	std::cout << "Boundary cycles: " << boundary_cycle
		<< " boundary vertices: " << boundary_size << std::endl;


	if(boundary_size < 3) {
		std::cerr << "Error: Boundary size is less than 3!" << std::endl;
		return;
	}
	Eigen::SparseMatrix<double> A(size, size);// ��һ������������

	Eigen::VectorXd b_x = Eigen::VectorXd::Zero(size);

	Eigen::VectorXd b_y = Eigen::VectorXd::Zero(size);

	Eigen::VectorXd pos_x = Eigen::VectorXd::Zero(size);
	Eigen::VectorXd pos_y = Eigen::VectorXd::Zero(size);

	int column = 0;
	int count = 0;// ��¼�����С
	for (int i = 0; i < size; i++) {
		if (mesh.vertices[i]->isBoundary()) {

			//double x = (double)mesh.vertices[i]->boundary_index / (double)boundary_size * 2.0;//�����������
			//double y = 0;

			//if (x < 1) {
			//	y = 1 - sqrt(1 - (x - 1) * (x - 1));
			//}
			//else
			//	y = y = 1 + sqrt(1 - (x - 1) * (x - 1));
			//int index = mesh.vertices[i]->index;
			//// t �� 0 - 2pi��, Բ�ı߽���0 - 2pi
			//A.coeffRef(column, index) = 1;
			//b_x(column) = x;// ���� x ����
			//b_y(column) = y;// ���� y ����

			double t = (double)mesh.vertices[i]->boundary_index / (double)boundary_size;//����������� [0, B-1], B=boundary_size
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
		// �ڲ��ĵ�
		else {
			geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
			count = 0;
			do {

				A.coeffRef(column, hf->pair->vertex->index) = -1;
				hf = hf->pair->next;// ������һ������
				// i �У�
				count++;
			} while (hf != mesh.vertices[i]->halfEdge);
		}
		A.coeffRef(column, mesh.vertices[i].get()->index) = count;
		column++;
	}

	// ��ʼ������
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

    // ���� boundary_index
    for (auto& v : mesh.vertices) v->boundary_index = -1;

    // �ռ����б߽绷
    std::vector<std::vector<geometry::HalfEdge*>> loops;
    std::unordered_set<geometry::HalfEdge*> visitedHE;

    for (auto& hePtr : mesh.halfEdges) {
        geometry::HalfEdge* start = hePtr.get();
        if (start->pair) continue;                // ֻ�ӱ߽��߿�ʼ
        if (visitedHE.count(start)) continue;

        std::vector<geometry::HalfEdge*> loop;
        geometry::HalfEdge* he = start;
        int guard = 0;
        do {
            loop.push_back(he);// ֱ�����Ͱ�ߣ����Ϊ��ǰ��
            visitedHE.insert(he);
            // ����һ���߽���
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

    // �����б߽綥����������� global boundary_index������¼ÿ���ֲ�˳��
    int globalBoundaryCount = 0;
    struct LoopInfo {
        std::vector<geometry::Vertex*> verts;
    };
    std::vector<LoopInfo> loopInfos;
    loopInfos.reserve(loops.size());

    for (size_t li = 0; li < loops.size(); ++li) {
        LoopInfo info;
        auto& loopHEs = loops[li];
        // �������㶥��˳��
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

    // Ԥ��ϡ�����
    Eigen::SparseMatrix<double> A(n, n);
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(n * 6);

    Eigen::VectorXd bx = Eigen::VectorXd::Zero(n);
    Eigen::VectorXd by = Eigen::VectorXd::Zero(n);

    // �໷��ͬ��ԲǶ�루�뾶�ݼ���
    // �⻷�뾶 1.0������ r = 1.0 - 0.3 * 

    int up = 2;
    for (size_t li = 0; li < loopInfos.size(); ++li) {
       /* double r = 1.0 - 0.3 * li;*/
        double r = 1.0; // Բ�İ뾶
        double a = up * li;// �������̳���ϵ��
        if (r <= 0.05) r = 0.05; // ��ֹ̫С
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

    // �ڲ����㣺Uniform Laplacian
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
            if (!he->pair) { // ʵ���Ǳ߽綥�㣬����ê��
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
    // ������ LSCM: �����˻���һ��ֱ��
    // �˻�����ԭ������������չ���� y �����ӽ� 0 (����/����)����Լ��ֻ��һ�����ϡ�
    // �ȶ����ԣ�
    // 1. ��ȷ����ʽ���� z_j, z_k չ�� (i Ϊԭ��, j �� x ��)��
    // 2. �� y_k �������С���÷��߷�������һ����С��ֵ(���ֹ��νṹ)��
    // 3. ����ê����� (0,0) �� (1,0)�������������ţ���������ѹ����
    // 4. �淽��ʹ�õ�λȨ�����Ȩ (���������Ȩ�����Ƚ���)��

    int n = (int)mesh.vertices.size();
    int f = (int)mesh.faces.size();
    if (n == 0 || f == 0) return;

    // ��¼ԭʼ 3D �� old_position (��֮ǰδд��)��
    for (auto& v : mesh.vertices) v->old_position = v->position;

    // ѡ�����߽�ê��: ��Զ���� (���߽粻������������)
    int anchorA = -1, anchorB = -1;
    for (int i = 0; i < n; ++i) if (mesh.vertices[i]->isBoundary()) { anchorA = i; break; }
    if (anchorA >= 0) {
        double maxD = -1.0; Eigen::Vector3d pA = mesh.vertices[anchorA]->old_position;
        for (int i = 0; i < n; ++i) if (i != anchorA && mesh.vertices[i]->isBoundary()) {
            double d = (mesh.vertices[i]->old_position - pA).norm(); if (d > maxD) { maxD = d; anchorB = i; }
        }
    }
    if (anchorA < 0 || anchorB < 0) { // ���㹻�߽�
        anchorA = 0; anchorB = (n > 1 ? 1 : 0); double maxD = -1.0;
        for (int i = 0; i < n; i++)for (int j = i + 1; j < n; j++) { double d = (mesh.vertices[i]->old_position - mesh.vertices[j]->old_position).norm(); if (d > maxD) { maxD = d; anchorA = i; anchorB = j; } }
    }
    if (anchorA == anchorB) return;

    // ϵͳ��2*f + 4 �У�2*n �� (u(0..n-1), v(0..n-1))
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
        if (Lij < 1e-14 || Lik < 1e-14) continue; // �˻�����

        // չ��: j �� (Lij,0)
        double xk = e_ik.dot(e_ij) / Lij; // k �� e_ij ����ͶӰ
        double yk2 = std::max(0.0, e_ik.squaredNorm() - xk * xk);
        double yk = (yk2 <= 1e-20 ? 1e-10 : std::sqrt(yk2)); // ��С���һ����ֵ��ֹ��Ƭ����ֱ��

        // r = (z_j - z_i)/(z_k - z_i) = Lij / (xk + i yk)
        std::complex<double> zj(Lij, 0.0); std::complex<double> zk(xk, yk);
        if (std::abs(zk) < 1e-14) continue; // �˻�
        std::complex<double> r_c = zj / zk;
        double rRe = r_c.real();
        double rIm = r_c.imag();

        // �����Ȩ�� (��ԭ 3D ���)
        double area2 = e_ij.cross(e_ik).norm();
        double w = 0.5 * area2;
        // Լ��1: (u_j-u_i) - rRe*(u_k-u_i) + rIm*(v_k-v_i) = 0
        triplets.emplace_back(r, uId(j), w * 1.0);
        triplets.emplace_back(r, uId(i), w * -1.0);
        triplets.emplace_back(r, uId(k), w * -rRe);
        triplets.emplace_back(r, uId(i), w * rRe); // �ӻ� rRe*u_i
        triplets.emplace_back(r, vId(k), w * rIm);
        triplets.emplace_back(r, vId(i), w * -rIm);
        r++;
        // Լ��2: (v_j-v_i) - rRe*(v_k-v_i) - rIm*(u_k-u_i) = 0
        triplets.emplace_back(r, vId(j), w * 1.0);
        triplets.emplace_back(r, vId(i), w * -1.0);
        triplets.emplace_back(r, vId(k), w * -rRe);
        triplets.emplace_back(r, vId(i), w * rRe);
        triplets.emplace_back(r, uId(k), w * -rIm);
        triplets.emplace_back(r, uId(i), w * rIm);
        r++;
    }

    // ê��Լ��: uA=0, vA=0, uB=1, vB=0
    triplets.emplace_back(r, uId(anchorA), 1.0); b[r] = 0.0; r++;
    triplets.emplace_back(r, vId(anchorA), 1.0); b[r] = 0.0; r++;
    triplets.emplace_back(r, uId(anchorB), 1.0); b[r] = 1.0; r++;
    triplets.emplace_back(r, vId(anchorB), 1.0); b[r] = 0.0; r++;

    int finalRows = r;
    Eigen::SparseMatrix<double> M(finalRows, cols);
    M.setFromTriplets(triplets.begin(), triplets.end());
    Eigen::VectorXd b_use = b.head(finalRows);

    // ��С����: (M^T M) x = M^T b
    Eigen::SparseMatrix<double> MtM = M.transpose() * M;
    Eigen::VectorXd MtB = M.transpose() * b_use;
    for (int i = 0; i < cols; i++) MtM.coeffRef(i, i) += 1e-10; // ����

    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver; solver.compute(MtM);
    if (solver.info() != Eigen::Success) { std::cerr << "LSCM: factorization failed" << std::endl; return; }
    Eigen::VectorXd sol = solver.solve(MtB);
    if (solver.info() != Eigen::Success) { std::cerr << "LSCM: solve failed" << std::endl; return; }

    for (int i = 0; i < n; ++i) { double u = sol[uId(i)], v = sol[vId(i)]; mesh.vertices[i]->position = { (float)u,(float)v,0.0f }; }
}