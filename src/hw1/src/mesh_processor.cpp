#include "mesh_processor.h"
#include <mesh_converter.h>
#include <iostream>

std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
MeshProcessor::processOBJData(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices) {
    
    // ����1��ʹ��geometryģ���MeshConverter�����������
    geometry::MeshConverter::buildMeshFromQtData(mesh, vertices, indices);

    // ����2����֤��߽ṹ����ȷ��
    if (!mesh.isValid()) {
        std::cerr << "Warning: Generated half-edge mesh is invalid!" << std::endl;
    }

    // ����3��ִ��ʵ�ʵļ��δ������������Ҫ�Լ�ʵ�ֵĲ��֣�
    std::vector<int> indexes = {1, 2, 3, 4};
    processGeometry(indexes);
    

    // ����4��ʹ��geometryģ���MeshConverter�����ת��Qt��ʽ
    return geometry::MeshConverter::convertMeshToQtData(mesh);
}
// work1 �ҵ����������������֮������·��
// ������points Ӧ���ǵ����ţ������ǵ������
// cubeģ�Ͳ��У������Ƿ�վͲ���
void MeshProcessor::processGeometry(std::vector<int>& indexe) {

	std::vector<std::vector<double>> res(mesh.vertices.size(), std::vector<double>(mesh.vertices.size(), 0));

    std::cout << "==== homework 1 Started ====" << std::endl;
    std::cout << "Vertices: " << mesh.vertices.size() << std::endl;
    std::cout << "Faces: " << mesh.faces.size() << std::endl;

    // ��ʼ�����ж������ɫΪ��ɫ
    //for (auto& vertex : mesh.vertices) {
    //    vertex->color = Eigen::Vector3d(1.0, 1.0, 1.0); // White
    //}

    //int point_size = indexes.size();
  //  if (point_size < 2) {
		//std::cout << "Please select at least two points." << std::endl;
  //  }
    int point_size = mesh.vertices.size();
    //���濪ʼ�ҳ�ÿ������֮������·��
    // ��Ҫʹ�÷�յ�ģ�ͣ�����v -> halfedge -> pair -> next���ܻ���ֿ�ָ��
    for (int i = 0; i < point_size; i++) {
		int start_index = i;
        std::vector<double> res_now = dijkstra(start_index, mesh);
		res[start_index] = res_now;
        res_now.clear();
    }
    std::cout << "==== Complete Graph Constructed ====" << std::endl;

    // ����MST
	std::vector<std::vector<double>> mst(point_size, std::vector<double>(point_size, 0));
	std::vector<bool> in_mst(point_size, false);// ��¼��Щ���Ѿ���MST��
    std::vector<int> parent(point_size, -1);      // �洢 MST �еĸ��ڵ�
    std::vector<double> key(point_size, INT_MAX); // ��С��������
	// ����Prim�㷨����MST
    // ����stack�洢û�з��ʵĶ���
    key[0] = 0; // �ӵ�һ���㣨ȫ������ 0����ʼ
    for (int count = 0; count < point_size; count++) {
        int u = -1;

		double min_val = INT_MAX;// �ҵ� key ֵ��С�ĵ� u

        for (int i = 0; i < point_size; i++) {
            if (!in_mst[i] && key[i] < min_val) {
                min_val = key[i];
                u = i;
            }
		}// �����ҵ���prim����һ����u

        if (u == -1) break; // ���еĵ㶼������
        in_mst[u] = true; // �� u ���� MST ����

        // 2. ��¼ MST �ߣ��������Ҫ���ӻ���
        if (parent[u] != -1) {
            // ���� parent[u] �� u�������� key[u]
            mst[parent[u]][u] = key[u];
            //mst[u][parent[u]] = key[u];
            std::cout << "Edge added: " << parent[u] << " - " << u << " with distance " << key[u] << std::endl;
        }

        // 3. ���� u �������ھ� v �� key ֵ��**�������Ų���**��
        for (int v = 0; v < point_size; v++) {
            // res[u][v] �� u �� v �ľ���
            if (!in_mst[v] && res[u][v] < key[v]) {
                // ÿ��ֱ�ӱ�����һ��ȡ�õĶ��㣬
                parent[v] = u; // ��¼ u Ϊ v �ĸ��ڵ�
                key[v] = res[u][v]; // ������С���Ӿ���
            }
        }
    }
	// MST�������
	// ���棬�������Щ��С�������ı�����Ϊ��ɫ����Ӧ�ص�ͼ�е�������ȥ
    // ������һ����ô��
    for(int i = 0; i < point_size; i++) {
        for (int j = i + 1; j < point_size; j++) {
            if (mst[i][j] != 0 || mst[j][i] != 0) {
                // i �� j ֮����һ��MST�ߣ������ǵĶ�����ɫ����Ϊ��ɫ
                if (i < mesh.vertices.size() && j < mesh.vertices.size()) {
                    mesh.vertices[i]->color = Eigen::Vector3d(1.0, 0.0, 0.0); // Red
                    mesh.vertices[j]->color = Eigen::Vector3d(1.0, 0.0, 0.0); // Red
                }
            }
        }
    }

    std::cout << "==== Homework 1  Completed ====" << std::endl;
}


const double INF = std::numeric_limits<double>::infinity();

std::vector<double> MeshProcessor::dijkstra(int start_index, geometry::HalfEdgeMesh& mesh) {
    const int size = mesh.vertices.size();
    std::vector<bool> visited(size, false);// ��������
    std::vector<double> dist(size, INF); //�����ʼ��Ϊ�����
    dist[start_index] = 0; //������Ϊ0

    for (int i = 0; i < size; i++) {
		int u = find_min_distance_node(dist, visited);// ��һ��������С�Ķ���
        if (u == -1) break; //���нڵ㶼���ʹ���
		// �����һ��һ�����ҵ�start_index
		visited[u] = true; //���Ϊ�ѷ���

        // ����u�������ھ�, ��һ������ʼ����
		geometry::HalfEdge* current_halfedge = mesh.vertices[u]->halfEdge;

        do{
			int v = current_halfedge->getEndVertex()->index; //�ھӽڵ�����
            double weight = current_halfedge->getLength();// ����Ȩ��

            if(dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight; //���¾���
			}

            if (current_halfedge->isBoundary()) {
                break;
            }
			current_halfedge = current_halfedge->pair->next; //��һ���ھ�
        } while (current_halfedge != mesh.vertices[u]->halfEdge);
		// ����Ѿ����ʵ����յ㣬������ǰ����   ���ﻹһ��Ҫdo while
    }
	return dist;

}



// ������������δ���ʵĽڵ����ҵ�������С�Ľڵ�
int MeshProcessor::find_min_distance_node(
    const std::vector<double>& dist,
    const std::vector<bool>& visited)
{
    double min_dist = INF;
    int min_index = -1;

    // �������нڵ�
    for (size_t i = 0; i < dist.size(); ++i) {
        // ������δ���ʹ��ģ����Ҿ���С�ڵ�ǰ��Сֵ
        if (!visited[i] && dist[i] < min_dist) {
            min_dist = dist[i];
            min_index = (int)i;
        }
    }
    return min_index;
}