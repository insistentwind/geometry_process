#include "mesh_processor.h"
#include <mesh_converter.h>
#include <iostream>
#include <Eigen/Core>
#include <unordered_set>
#include <Eigen/dense>
#include <Eigen/Sparse>


std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
MeshProcessor::processOBJData(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices) {
    
    // ����1��ʹ��geometryģ���MeshConverter�����������
    geometry::MeshConverter::buildMeshFromQtData(mesh, vertices, indices);

    // ����2����֤��߽ṹ����ȷ��
    if (!mesh.isValid()) {
        std::cerr << "Warning: Generated half-edge mesh is invalid!" << std::endl;
    }

    // �м䲽�裺�ҳ�tuttes embedding��Ķ�������
    tuttes_embedding();
	//LSCM();

	// tuttes��֮�����е�ԭʼ��������old_position�У��������Ķ������position��
	
    // ����3��ִ��ʵ�ʵļ��δ��������������Ҫ�Լ�ʵ�ֵĲ��֣�
    processGeometry();


    // ����4��ʹ��geometryģ���MeshConverter�����ת��Qt��ʽ
    return geometry::MeshConverter::convertMeshToQtData(mesh);
}


void MeshProcessor::processGeometry() {
	int f_size = mesh.faces.size();

	Eigen::MatrixXd local_coords; // ÿ��face�ľֲ�����
	local_coords.resize(f_size, 4);// ÿ��ƽ��ֲ����� ��ԭʼ��������ƽ���6������

	int boundary_count = 0;
	for (int i = 0; i < mesh.vertices.size(); i++) {
		if(mesh.vertices[i]->isBoundary()) {
			boundary_count++;
		}
	}

  // �������jacob����


	for (int i = 0; i < f_size; i++) {
		auto face = mesh.faces[i].get();

		auto he = face->halfEdge;
		//std::vector<Eigen::Vector3d> points(3);
		//do {
		//	Eigen::Vector3d point = he->vertex->position;
		//	points[i] = point;
		//	he = he->next;
		//} while (he != face->halfEdge);
		
		//������points�����õ�ÿ������д���
		auto p0 = he->vertex->old_position;
		auto p1 = he->next->vertex->old_position;
		auto p2 = he->next->next->vertex->old_position;

		auto p0p1 = p1 - p0;
		auto p0p2 = p2 - p0;

		double e1 = p0p1.norm();
		double e2 = p0p2.norm();//��ȡֵ

		double x = p0p1.dot(p0p2) / e1;
		double y = sqrt(std::max(0.0, e2 * e2 - x * x)); // �������⸺����ȡƽ����

		local_coords(i, 0) = e1;
		local_coords(i, 1) = 0;
		local_coords(i, 2) = x;
		local_coords(i, 3) = y;
	}


	std::vector<Eigen::Matrix2d> global(f_size);// ȫ�ֱ任����
	// ����ָ���ÿ��������ƽ���rotation
	for (int i = 0; i < f_size; i++) {
		Eigen::Matrix2d J, S, D;
		
		geometry::HalfEdge* he = mesh.faces[i]->halfEdge;

		Eigen::Vector3d p0, p1, p2;

		p0 = he->vertex->position;//�������������
		he = he->next;
		p1 = he->vertex->position;
		he = he->next;
		p2 = he->vertex->position;


		S << local_coords(i, 0), local_coords(i, 2),
			local_coords(i, 1), local_coords(i, 3);// ��ά�е�����

		D << p1(0) - p0(0), p2(0) - p0(0),
			p1(1) - p0(1), p2(1) - p0(1);// ��ά�е�����
	/*	D << p2(0) - p0(0), p1(0) - p0(0), 
		p2(1) - p0(1), p1(1) - p0(1);*/

		J = D * S.inverse();

		Eigen::Matrix2d rotation;
		// �Ծ���J����SVD�ֽ��õ�rotation

		Eigen::JacobiSVD<Eigen::Matrix2d> svd(J, Eigen::ComputeFullU | Eigen::ComputeFullV);

		svd.singularValues();

		rotation = svd.matrixU() * svd.matrixV().transpose();

		if(rotation.determinant() < 0) {
			Eigen::Matrix2d V = svd.matrixV();
			V.col(1) *= -1;
			rotation = svd.matrixU() * V.transpose();
		}
		global[i] = rotation;
	}

	// ����cotangentȨֵ
	std::vector<double> cots(mesh.halfEdges.size(), 0.0);//ΪʲôҪ���������

	int v_size = mesh.vertices.size();

	Eigen::SparseMatrix<double> A(v_size, v_size);// ��һ������������

	std::vector<Eigen::Triplet<double>> trivec;
	//���濪ʼglobal��������µ� jacob�����õ�������
	// ����cotanget laplace����
	for (int i = 0; i < mesh.halfEdges.size(); i++)
	{
		auto ithe = mesh.halfEdges[i].get();
		mesh.halfEdges[i]->index = i;

		//if (ithe->isBoundary())
		//	continue;

		//auto v0 = ithe->prev->vertex->old_position;
		auto v0 = ithe->next->next->vertex->old_position;
		auto v2 = ithe->next->vertex->old_position;
		auto v1 = ithe->vertex->old_position;

		auto v0v1 = v1 - v0;
		auto v0v2 = v2 - v0;
		//cot �� ������߶��ŵ��Ǹ���
		double crossNorm = v0v1.cross(v0v2).norm();
		//if (crossNorm < 1e-14) continue; // �����˻������Σ�����cot��ը
		double cotangent = v0v1.dot(v0v2) / crossNorm;

		// ====================== �� �ؼ����� �� ======================
	// ��cotangentֵ���нضϣ���ֹ�����������ε�����ֵ��ը
		const double max_cot_abs = 1.0e2; // ����һ������ľ���ֵ����, e.g., 100,000
		cotangent = std::max(-max_cot_abs, std::min(max_cot_abs, cotangent));
		// ==========================================================


		cots[i] = cotangent;

		int ve0 = ithe->vertex->index;
		int ve1 = ithe->getEndVertex()->index;
		//A.coeffRef(ve0, ve1) -= cotangent;
		//A.coeffRef(ve1, ve0) -= cotangent;
		//A.coeffRef(ve1, ve1) += cotangent;
		//A.coeffRef(ve0, ve0) += cotangent;
		trivec.emplace_back(ve0, ve0, cotangent);
		trivec.emplace_back(ve1, ve1, cotangent);
		trivec.emplace_back(ve0, ve1, -cotangent);// ���ھӽڵ���Ϊ��ֵ���Լ�����ֵ
		trivec.emplace_back(ve1, ve0, -cotangent);
	}
	//���濪ʼglobal����
	// �ⲽҪ��ʼ���þ���ָ�������rotation����������ľ���jacob�任������
	// ��ԭʼ�������ν���rotation�任
	//Eigen::VectorXd b_x = Eigen::VectorXd::Zero(v_size + boundary_count);
	Eigen::VectorXd b_x = Eigen::VectorXd::Zero(v_size);
	Eigen::VectorXd b_y = Eigen::VectorXd::Zero(v_size);

	for (int i = 0; i < f_size; i++)
	{
		auto f = mesh.faces[i].get();
		auto he = f->halfEdge;

		//auto i2 = he->vertex->index;//��ʼ��
		//he = he->next;
		//auto i0 = he->vertex->index;
		//he = he->next;
		//auto i1 = he->vertex->index;

		//auto he2 = f->halfEdge;
		//auto he0 = he2->next;
		//auto he1 = he0->next;

		auto i0 = he->vertex->index;//��ʼ��
		he = he->next;
		auto i1 = he->vertex->index;
		he = he->next;
		auto i2 = he->vertex->index;

		auto he0 = f->halfEdge;
		auto he1 = he0->next;
		auto he2 = he1->next;

		if (he0->index >= (int)cots.size() || he1->index >= (int)cots.size() || he2->index >= (int)cots.size()) continue; // ����

		Eigen::Vector2d e01, e12, e21;
		// ��Ӧ he0 (i0->i1)
		e01 << local_coords(i, 0), local_coords(i, 1);// e1 0
		// ��Ӧ he1 (i1->i2)
		e12 << local_coords(i, 2) - local_coords(i, 0), local_coords(i, 3) - local_coords(i, 1);
		// ��Ӧ he2 (i2->i0)
		e21 << -local_coords(i, 2), -local_coords(i, 3);

		Eigen::Vector2d b0 = cots[he0->index] * global[i] * e01;//���������ڲ�����
		b_x[i0] -= b0[0];
		b_y[i0] -= b0[1];

		b_x[i1] += b0[0];
		b_y[i1] += b0[1];
		Eigen::Vector2d b1 = cots[he1->index] * global[i] * e12;
		b_x[i1] -= b1[0];
		b_y[i1] -= b1[1];

		b_x[i2] += b1[0];
		b_y[i2] += b1[1];

		Eigen::Vector2d b2 = cots[he2->index] * global[i] * e21;
		b_x[i2] -= b2[0];
		b_y[i2] -= b2[1];

		b_x[i0] += b2[0];
		b_y[i0] += b2[1];
	}
	//����Լ��
	//int count = 0;
	//for(int i = 0; i < mesh.vertices.size(); i++) {
	//	if(mesh.vertices[i]->isBoundary()) {
	//		int row = v_size + count;
	//		int col = mesh.vertices[i]->index;
	//		trivec.emplace_back(row, col, 1.0);
	//		//b_x.conservativeResize(v_size);
	//		//b_y.conservativeResize(v_size);
	//		b_x[row] = mesh.vertices[i]->old_position.x();
	//		b_y[row] = mesh.vertices[i]->old_position.y();
	//		count++;
	//	}
	//}
	A.setFromTriplets(trivec.begin(), trivec.end());

	//[����] ΪARAP��A����ʩ�ӡ�ӲԼ�������������׼�����Ƚ��ķ���
	for (int i = 0; i < v_size; ++i) {
		if (mesh.vertices[i]->isBoundary()) {
			A.coeffRef(i, i) += 1e-8;
		}
	}

	//for (int i = 0; i < v_size; ++i) A.coeffRef(i, i) += 1e-8; // ��С�䶯1: �Խ����򣬻���������˹����
	Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
	//solver.compute(A);
	solver.compute(A);

	if (solver.info() != Eigen::Success) { // ��С�䶯2: �ֽ�ʧ����������
		std::cerr << "processGeometry: factorization failed" << std::endl;
		return;
	}


	Eigen::MatrixXd uv(v_size, 2);
	//uv.col(0) = solver.solve(b_x);
	
	uv.col(0) = solver.solve(b_x);
	if (solver.info() != Eigen::Success) { std::cerr << "processGeometry: solve b_x failed" << std::endl; return; } // ��С�䶯3: ���ʧ�ܼ��
	//uv.col(1) = solver.solve(b_y);
	uv.col(1) = solver.solve(b_y);
	if (solver.info() != Eigen::Success) { std::cerr << "processGeometry: solve b_y failed" << std::endl; return; } // ��С�䶯4


	for (int i = 0; i < mesh.vertices.size(); i++)
	{
		auto v_h = mesh.vertices[i].get();
		//v_h->old_position = v_h->position;
		v_h->position = { uv(i, 0), uv(i, 1), 0 };
	}

}







// tuttes embedding
void MeshProcessor::tuttes_embedding() {

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


	if (boundary_size < 3) {
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
		mesh.vertices[i]->old_position = mesh.vertices[i]->position; // ��¼��λ��

		if (mesh.vertices[i]->isBoundary()) {

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
    for (auto &v : mesh.vertices) v->old_position = v->position;

    // ѡ�����߽�ê��: ��Զ���� (���߽粻������������)
    int anchorA=-1, anchorB=-1;
    for (int i=0;i<n;++i) if (mesh.vertices[i]->isBoundary()) { anchorA=i; break; }
    if (anchorA>=0) {
        double maxD=-1.0; Eigen::Vector3d pA=mesh.vertices[anchorA]->old_position;
        for (int i=0;i<n;++i) if (i!=anchorA && mesh.vertices[i]->isBoundary()) {
            double d=(mesh.vertices[i]->old_position-pA).norm(); if (d>maxD){maxD=d;anchorB=i;}
        }
    }
    if (anchorA<0 || anchorB<0) { // ���㹻�߽�
        anchorA=0; anchorB=(n>1?1:0); double maxD=-1.0;
        for(int i=0;i<n;i++)for(int j=i+1;j<n;j++){double d=(mesh.vertices[i]->old_position-mesh.vertices[j]->old_position).norm();if(d>maxD){maxD=d;anchorA=i;anchorB=j;}} }
    if (anchorA==anchorB) return;

    // ϵͳ��2*f + 4 �У�2*n �� (u(0..n-1), v(0..n-1))
    int rows = 2*f + 4;
    int cols = 2*n;
    std::vector<Eigen::Triplet<double>> triplets;
    Eigen::VectorXd b = Eigen::VectorXd::Zero(rows);

    auto uId=[&](int vi){return vi;};
    auto vId=[&](int vi){return vi+n;};

    int r=0;
    for (int fi=0; fi<f; ++fi) {
        auto face = mesh.faces[fi].get();
        auto he0 = face->halfEdge; auto he1=he0->next; auto he2=he1->next;
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
        double yk2 = std::max(0.0, e_ik.squaredNorm() - xk*xk);
        double yk = (yk2 <= 1e-20 ? 1e-10 : std::sqrt(yk2)); // ��С���һ����ֵ��ֹ��Ƭ����ֱ��

        // r = (z_j - z_i)/(z_k - z_i) = Lij / (xk + i yk)
        std::complex<double> zj(Lij,0.0); std::complex<double> zk(xk, yk);
        if (std::abs(zk) < 1e-14) continue; // �˻�
        std::complex<double> r_c = zj / zk;
        double rRe = r_c.real();
        double rIm = r_c.imag();

        // �����Ȩ�� (��ԭ 3D ���)
        double area2 = e_ij.cross(e_ik).norm();
        double w = 0.5 * area2;
        // Լ��1: (u_j-u_i) - rRe*(u_k-u_i) + rIm*(v_k-v_i) = 0
        triplets.emplace_back(r,uId(j), w*1.0);
        triplets.emplace_back(r,uId(i), w*-1.0);
        triplets.emplace_back(r,uId(k), w*-rRe);
        triplets.emplace_back(r,uId(i), w*rRe); // �ӻ� rRe*u_i
        triplets.emplace_back(r,vId(k), w*rIm);
        triplets.emplace_back(r,vId(i), w*-rIm);
        r++;
        // Լ��2: (v_j-v_i) - rRe*(v_k-v_i) - rIm*(u_k-u_i) = 0
        triplets.emplace_back(r,vId(j), w*1.0);
        triplets.emplace_back(r,vId(i), w*-1.0);
        triplets.emplace_back(r,vId(k), w*-rRe);
        triplets.emplace_back(r,vId(i), w*rRe);
        triplets.emplace_back(r,uId(k), w*-rIm);
        triplets.emplace_back(r,uId(i), w*rIm);
        r++;
    }

    // ê��Լ��: uA=0, vA=0, uB=1, vB=0
    triplets.emplace_back(r,uId(anchorA),1.0); b[r]=0.0; r++;
    triplets.emplace_back(r,vId(anchorA),1.0); b[r]=0.0; r++;
    triplets.emplace_back(r,uId(anchorB),1.0); b[r]=1.0; r++;
    triplets.emplace_back(r,vId(anchorB),1.0); b[r]=0.0; r++;

    int finalRows = r;
    Eigen::SparseMatrix<double> M(finalRows, cols);
    M.setFromTriplets(triplets.begin(), triplets.end());
    Eigen::VectorXd b_use = b.head(finalRows);

    // ��С����: (M^T M) x = M^T b
    Eigen::SparseMatrix<double> MtM = M.transpose()*M;
    Eigen::VectorXd MtB = M.transpose()*b_use;
    for(int i=0;i<cols;i++) MtM.coeffRef(i,i) += 1e-10; // ����

    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver; solver.compute(MtM);
    if (solver.info()!=Eigen::Success){ std::cerr << "LSCM: factorization failed" << std::endl; return; }
    Eigen::VectorXd sol = solver.solve(MtB);
    if (solver.info()!=Eigen::Success){ std::cerr << "LSCM: solve failed" << std::endl; return; }

    for (int i=0;i<n;++i){ double u=sol[uId(i)], v=sol[vId(i)]; mesh.vertices[i]->position={ (float)u,(float)v,0.0f }; }
}