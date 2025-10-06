#include "mesh_processor.h"
#include <mesh_converter.h>
#include <iostream>

std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
MeshProcessor::processOBJData(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices) {
    
    // 步骤1：使用geometry模块的MeshConverter构建半边网格
    geometry::MeshConverter::buildMeshFromQtData(mesh, vertices, indices);

    // 步骤2：验证半边结构的正确性
    if (!mesh.isValid()) {
        std::cerr << "Warning: Generated half-edge mesh is invalid!" << std::endl;
    }

    // 步骤3：执行实际的几何处理操作（这需要自己实现的部分）
    std::vector<int> indexes = {1, 2, 3, 4};
    processGeometry(indexes);
    

    // 步骤4：使用geometry模块的MeshConverter将结果转回Qt格式
    return geometry::MeshConverter::convertMeshToQtData(mesh);
}
// work1 找到任意给的两个顶点之间的最短路径
// 给出的points 应该是点的序号，而不是点的坐标

void MeshProcessor::processGeometry(std::vector<int>& indexes) {

    std::cout << "==== homework 1 Started ====" << std::endl;
    std::cout << "Vertices: " << mesh.vertices.size() << std::endl;
    std::cout << "Faces: " << mesh.faces.size() << std::endl;

    int point_size = indexes.size();
    if (point_size < 2) {
		std::cout << "Please select at least two points." << std::endl;
    }
    //下面开始找出每两个点之间的最短路径
    // 需要使用封闭的模型，否则v -> halfedge -> pair -> next可能会出现空指针
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
    std::vector<bool> visited(size, false);// 访问数组
    std::vector<double> dist(size, INT_MAX); //距离初始化为无穷大
    dist[start_index] = 0; //起点距离为0

    for (int i = 0; i < size; i++) {
		int u = find_min_distance_node(dist, visited);// 下一个距离最小的顶点
        if (u == -1) break; //所有节点都访问过了
		// 这里第一次一定会找到start_index
		visited[u] = true; //标记为已访问

        // 遍历u的所有邻居, 从一阶邻域开始遍历
		geometry::HalfEdge* current_halfedge = mesh.vertices[u]->halfEdge;

        do{
			int v = current_halfedge->getEndVertex()->index; //邻居节点索引
            double weight = current_halfedge->getLength();// 距离权重

            if(dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight; //更新距离
			}

            if (current_halfedge->isBoundary()) {
                break;
            }
			current_halfedge = current_halfedge->pair->next; //下一个邻居
        } while (current_halfedge != mesh.vertices[u]->halfEdge);
		// 如果已经访问到了终点，可以提前结束   这里还一定要do while
    }
	return dist[end_index];

}
const double INF = std::numeric_limits<double>::infinity();



// 辅助函数：在未访问的节点中找到距离最小的节点
int MeshProcessor::find_min_distance_node(
    const std::vector<double>& dist,
    const std::vector<bool>& visited)
{
    double min_dist = INF;
    int min_index = -1;

    // 遍历所有节点
    for (size_t i = 0; i < dist.size(); ++i) {
        // 必须是未访问过的，并且距离小于当前最小值
        if (!visited[i] && dist[i] < min_dist) {
            min_dist = dist[i];
            min_index = (int)i;
        }
    }
    return min_index;
}