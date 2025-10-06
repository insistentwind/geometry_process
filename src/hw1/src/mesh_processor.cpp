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

void MeshProcessor::processGeometry(std::vector<int>& indexes) {

    std::cout << "==== homework 1 Started ====" << std::endl;
    std::cout << "Vertices: " << mesh.vertices.size() << std::endl;
    std::cout << "Faces: " << mesh.faces.size() << std::endl;

    int point_size = indexes.size();
    if (point_size < 2) {
		std::cout << "Please select at least two points." << std::endl;
    }
    //���濪ʼ�ҳ�ÿ������֮������·��
    // ��Ҫʹ�÷�յ�ģ�ͣ�����v -> halfedge -> pair -> next���ܻ���ֿ�ָ��
    for (int i = 0; i < point_size - 1; i++) {
        int start_index = indexes[i];
        for(int j = i + 1; j < point_size ; j++){
            int end_index = indexes[j];
            if (start_index < 0 || start_index >= mesh.vertices.size() ||
                end_index < 0 || end_index >= mesh.vertices.size())
                std::cout << "Point index out of range: " << std::endl;

            double shortest_length = dijkstra(start_index, end_index, mesh);
            std::cout << "Shortest path from vertex " << start_index
                << " to vertex " << end_index
                << " is " << shortest_length << std::endl;
		}
        
    }
  
    
    std::cout << "==== Homework 1 Completed ====" << std::endl;
}


double MeshProcessor::dijkstra(int start_index, int end_index, geometry::HalfEdgeMesh& mesh) {
    const int size = mesh.vertices.size();
    std::vector<bool> visited(size, false);// ��������
    std::vector<double> dist(size, INT_MAX); //�����ʼ��Ϊ�����
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
	return dist[end_index];

}
const double INF = std::numeric_limits<double>::infinity();



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