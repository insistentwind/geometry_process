#include "mesh_processor.h"
#include <mesh_converter.h>
#include <iostream>
#include <unordered_set>

/* --------------------------------------------------------------------------
 * MeshProcessor::processOBJData
 * ���: ԭʼ����/���� (Qt ��ʽ)
 * ����:
 *   1) ���� HalfEdgeMesh
 *   2) У��ṹ
 *   3) ������ҵ���δ��� (��ǰ: ���������� + MST ���)
 *   4) ת�� Qt ����/����
 * -------------------------------------------------------------------------- */
std::pair<std::vector<QVector3D>, std::vector<unsigned int>>
MeshProcessor::processOBJData(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices) {
    geometry::MeshConverter::buildMeshFromQtData(mesh, vertices, indices);
    if (!mesh.isValid()) {
        std::cerr << "Warning: Generated half-edge mesh is invalid!" << std::endl;
    }
    std::vector<int> placeholder = {1, 2, 3, 4}; // ��ǰδʹ��, Ԥ�������ӿ�
    processGeometry(placeholder);
    return geometry::MeshConverter::convertMeshToQtData(mesh);
}

/* ��ȡ������ɫ (��δ���㷨д�붥����ɫ) */
std::vector<QVector3D> MeshProcessor::extractColors() const {
    std::vector<QVector3D> cols; cols.reserve(mesh.vertices.size());
    for (const auto& vp : mesh.vertices) {
        if (vp) {
            const auto& c = vp->color;
            cols.emplace_back((float)c.x(), (float)c.y(), (float)c.z());
        } else {
            cols.emplace_back(1.f, 1.f, 1.f);
        }
    }
    return cols;
}

/* ɨ������ halfEdges, �� edgeColor==��, ��鲢Ϊ���� MST �� */
std::vector<std::pair<int, int>> MeshProcessor::extractMSTEdges() const {
    std::vector<std::pair<int, int>> result;
    result.reserve(mesh.halfEdges.size() / 2);
    std::unordered_set<long long> used;
    auto keyFn = [](int a, int b) {
		return ((long long)std::min(a, b) << 32) | (unsigned)std::max(a, b);// ������Ϊ������Ψһ����|������
    };
    for (const auto& heUP : mesh.halfEdges) {
        auto* he = heUP.get();
        if (!he || !he->vertex || !he->next) continue;
        if (he->edgeColor == Eigen::Vector3d(1, 0, 0)) {
            int a = he->vertex->index;
            int b = he->getEndVertex()->index;
            if (a == b) continue;
            long long k = keyFn(a, b);
            if (used.insert(k).second) {
                result.emplace_back(std::min(a, b), std::max(a, b));
            }
        }
    }
    return result;
}

/* --------------------------------------------------------------------------
 * ���ļ��δ���: �������е�����·������(res), ���� Prim ���� MST,
 * ���� MST �߶�Ӧ�� halfEdge.edgeColor ��Ϊ��ɫ.
 * -------------------------------------------------------------------------- */
void MeshProcessor::processGeometry(std::vector<int>& /*indexPlaceholder*/) {
    int n = static_cast<int>(mesh.vertices.size());
    if (n == 0) return;

    std::vector<std::vector<double>> res(n, std::vector<double>(n, 0.0));

    std::cout << "==== homework 1 Started ====\n";
    std::cout << "Vertices: " << mesh.vertices.size() << "\n";
    std::cout << "Faces   : " << mesh.faces.size()    << "\n";

    // 1. ��Դ���·�� (��ÿ��������һ�� dijkstra) ���� ���Ӷȸ�, ��ѧʾ��
    for (int i = 0; i < n; ++i) {
        res[i] = dijkstra(i, mesh);
    }
    std::cout << "==== Complete Graph Constructed ====\n";

    // 2. Prim �㷨���� MST (ʹ�� res ��Ϊ��ȫͼ����)
    std::vector<std::vector<double>> mst(n, std::vector<double>(n, 0.0));
	std::vector<bool> inMST(n, false);// ��¼ÿ���ڵ��Ƿ��Ѽ��� MST
	std::vector<int>  parent(n, -1);// ��¼ÿ���ڵ��� MST �еĸ��ڵ�
	std::vector<double> key(n, static_cast<double>(INT_MAX));// ��¼ÿ���ڵ㵽 MST ����С����

    key[0] = 0.0;

    for (int c = 0; c < n; ++c) {
        int u = -1; double minVal = static_cast<double>(INT_MAX);
        for (int i = 0; i < n; ++i) {
            if (!inMST[i] && key[i] < minVal) { minVal = key[i]; u = i; }
        }
        if (u == -1) break;
        inMST[u] = true;
        if (parent[u] != -1) {
            mst[parent[u]][u] = key[u];
            std::cout << "Edge added: " << parent[u] << " - " << u << " d=" << key[u] << "\n";
        }
        for (int v = 0; v < n; ++v) {
            if (!inMST[v] && res[u][v] < key[v]) {
                parent[v] = u;
                key[v] = res[u][v];
            }
        }
    }

    // 3. ���ö�����ɫΪ��
    for (auto& vp : mesh.vertices) if (vp) vp->color = Eigen::Vector3d(1, 1, 1);

    // 4. ���� MST ����, ��Ƕ�Ӧ halfEdge Ϊ��ɫ
    int marked = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (mst[i][j] != 0.0) {
                geometry::HalfEdge* he = mesh.vertices[i]->halfEdge;
                if (!he) continue;
                geometry::HalfEdge* start = he;
                do {
                    int endIdx = he->getEndVertex()->index;
                    if (endIdx == j) {
                        he->edgeColor = Eigen::Vector3d(1, 0, 0);
                        if (he->pair) he->pair->edgeColor = Eigen::Vector3d(1, 0, 0);
                        ++marked;
                        break;
                    }
                    if (he->isBoundary()) break;
                    he = he->pair ? he->pair->next : nullptr;
                } while (he && he != start);
            }
        }
    }

    std::cout << "MST half-edges colored: " << marked << "\n";
    std::cout << "==== Homework 1  Completed ====\n";
}

/* ��Դ���·����Dijkstra (���� half-edge �ڽ�) */
const double INF = std::numeric_limits<double>::infinity();

std::vector<double> MeshProcessor::dijkstra(int start, geometry::HalfEdgeMesh& m) {
    int n = static_cast<int>(m.vertices.size());
    std::vector<bool>    visited(n, false);
    std::vector<double>  dist(n, INF);
    dist[start] = 0.0;

    for (int iter = 0; iter < n; ++iter) {
		int u = find_min_distance_node(dist, visited);// δ�����о�����С�Ķ���
        if (u == -1) break;
        visited[u] = true;
        geometry::HalfEdge* he = m.vertices[u]->halfEdge;
		if (!he) continue;// ���ڽӱ�
        geometry::HalfEdge* startHe = he;

        do {
            int v = he->getEndVertex()->index;
            double w = he->getLength();
            if (dist[u] + w < dist[v]) dist[v] = dist[u] + w;
            if (he->isBoundary()) break; // ���߽粻�ٻ���
            he = he->pair ? he->pair->next : nullptr;
        } while (he && he != startHe);
    }
    return dist;
}

/* ����ɨ��: ѡȡδ�����о�����С�Ķ��� */
int MeshProcessor::find_min_distance_node(const std::vector<double>& dist,
                                          const std::vector<bool>& visited) {
    double minD = INF; int idx = -1;
    for (size_t i = 0; i < dist.size(); ++i) {
        if (!visited[i] && dist[i] < minD) { minD = dist[i]; idx = static_cast<int>(i); }
    }
    return idx;
}