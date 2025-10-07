#include "mesh_processor.h"
#include <mesh_converter.h>
#include <iostream>
#include <unordered_set>

std::pair<std::vector<QVector3D>, std::vector<unsigned int>>
MeshProcessor::processOBJData(const std::vector<QVector3D>& vertices,
	const std::vector<unsigned int>& indices) {
	geometry::MeshConverter::buildMeshFromQtData(mesh, vertices, indices);
	if (!mesh.isValid()) {
		std::cerr << "Warning: Generated half-edge mesh is invalid!" << std::endl;
	}
	std::vector<int> indexes = { 1, 2, 3, 4 };
	processGeometry(indexes);
	return geometry::MeshConverter::convertMeshToQtData(mesh);
}

std::vector<QVector3D> MeshProcessor::extractColors() const {
	std::vector<QVector3D> cols; cols.reserve(mesh.vertices.size());
	for (const auto& vp : mesh.vertices) {
		if (vp) { const auto& c = vp->color; cols.emplace_back((float)c.x(), (float)c.y(), (float)c.z()); }
		else cols.emplace_back(1.f, 1.f, 1.f);
	}
	return cols;
}

std::vector<std::pair<int, int>> MeshProcessor::extractMSTEdges() const {
	std::vector<std::pair<int, int>> result;
	result.reserve(mesh.halfEdges.size() / 2);
	std::unordered_set<long long> used;
	auto keyFn = [](int a, int b) { return ((long long)std::min(a, b) << 32) | (unsigned)std::max(a, b); };
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

void MeshProcessor::processGeometry(std::vector<int>& indexe) {
	std::vector<std::vector<double>> res(mesh.vertices.size(), std::vector<double>(mesh.vertices.size(), 0));
	std::cout << "==== homework 1 Started ====" << std::endl;
	std::cout << "Vertices: " << mesh.vertices.size() << std::endl;
	std::cout << "Faces: " << mesh.faces.size() << std::endl;
	int point_size = (int)mesh.vertices.size();
	for (int i = 0; i < point_size; i++) { auto dist = dijkstra(i, mesh); res[i] = std::move(dist); }
	std::cout << "==== Complete Graph Constructed ====" << std::endl;
	std::vector<std::vector<double>> mst(point_size, std::vector<double>(point_size, 0));
	std::vector<bool> in_mst(point_size, false); std::vector<int> parent(point_size, -1); std::vector<double> key(point_size, INT_MAX); key[0] = 0;
	for (int count = 0; count < point_size; count++) {
		int u = -1; double min_val = INT_MAX;
		for (int i = 0; i < point_size; i++) if (!in_mst[i] && key[i] < min_val) { min_val = key[i]; u = i; }
		if (u == -1) break; in_mst[u] = true;
		if (parent[u] != -1) { mst[parent[u]][u] = key[u]; std::cout << "Edge added: " << parent[u] << " - " << u << " d=" << key[u] << std::endl; }
		for (int v = 0; v < point_size; v++) if (!in_mst[v] && res[u][v] < key[v]) { parent[v] = u; key[v] = res[u][v]; }
	}
	for (auto& vp : mesh.vertices) if (vp) vp->color = Eigen::Vector3d(1, 1, 1);
	int marked = 0;
	for (int i = 0; i < point_size; ++i) {
		for (int j = 0; j < point_size; ++j) {
			if (mst[i][j] != 0) {
				geometry::HalfEdge* he = mesh.vertices[i]->halfEdge; if (he) { geometry::HalfEdge* start = he; do { int endIdx = he->getEndVertex()->index; if (endIdx == j) { he->edgeColor = Eigen::Vector3d(1, 0, 0); if (he->pair) he->pair->edgeColor = Eigen::Vector3d(1, 0, 0); marked++; break; } if (he->isBoundary()) break; he = he->pair ? he->pair->next : nullptr; } while (he && he != start); }
			}
		}
	}
	std::cout << "MST half-edges colored: " << marked << std::endl;
	std::cout << "==== Homework 1  Completed ====" << std::endl;
}

const double INF = std::numeric_limits<double>::infinity();
std::vector<double> MeshProcessor::dijkstra(int start_index, geometry::HalfEdgeMesh& mesh) {
	const int size = (int)mesh.vertices.size(); std::vector<bool> visited(size, false); std::vector<double> dist(size, INF); dist[start_index] = 0;
	for (int i = 0; i < size; i++) {
		int u = find_min_distance_node(dist, visited); if (u == -1) break; visited[u] = true; geometry::HalfEdge* he = mesh.vertices[u]->halfEdge; if (!he) continue; do { int v = he->getEndVertex()->index; double w = he->getLength(); if (dist[u] + w < dist[v]) dist[v] = dist[u] + w; if (he->isBoundary()) break; he = he->pair ? he->pair->next : nullptr; } while (he && he != mesh.vertices[u]->halfEdge);
	}
	return dist;
}

int MeshProcessor::find_min_distance_node(const std::vector<double>& dist, const std::vector<bool>& visited) { double md = INF; int idx = -1; for (size_t i = 0; i < dist.size(); ++i) if (!visited[i] && dist[i] < md) { md = dist[i]; idx = (int)i; } return idx; }