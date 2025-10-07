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
// cube模型不行，好像不是封闭就不行
void MeshProcessor::processGeometry(std::vector<int>& indexe) {

	std::vector<std::vector<double>> res(mesh.vertices.size(), std::vector<double>(mesh.vertices.size(), 0));

    std::cout << "==== homework 1 Started ====" << std::endl;
    std::cout << "Vertices: " << mesh.vertices.size() << std::endl;
    std::cout << "Faces: " << mesh.faces.size() << std::endl;

    // 初始化所有顶点的颜色为白色
    //for (auto& vertex : mesh.vertices) {
    //    vertex->color = Eigen::Vector3d(1.0, 1.0, 1.0); // White
    //}

    //int point_size = indexes.size();
  //  if (point_size < 2) {
		//std::cout << "Please select at least two points." << std::endl;
  //  }
    int point_size = mesh.vertices.size();
    //下面开始找出每两个点之间的最短路径
    // 需要使用封闭的模型，否则v -> halfedge -> pair -> next可能会出现空指针
    for (int i = 0; i < point_size; i++) {
		int start_index = i;
        std::vector<double> res_now = dijkstra(start_index, mesh);
		res[start_index] = res_now;
        res_now.clear();
    }
    std::cout << "==== Complete Graph Constructed ====" << std::endl;

    // 构建MST
	std::vector<std::vector<double>> mst(point_size, std::vector<double>(point_size, 0));
	std::vector<bool> in_mst(point_size, false);// 记录哪些点已经在MST中
    std::vector<int> parent(point_size, -1);      // 存储 MST 中的父节点
    std::vector<double> key(point_size, INT_MAX); // 最小距离数组
	// 利用Prim算法构建MST
    // 利用stack存储没有访问的顶点
    key[0] = 0; // 从第一个点（全局索引 0）开始
    for (int count = 0; count < point_size; count++) {
        int u = -1;

		double min_val = INT_MAX;// 找到 key 值最小的点 u

        for (int i = 0; i < point_size; i++) {
            if (!in_mst[i] && key[i] < min_val) {
                min_val = key[i];
                u = i;
            }
		}// 这里找到了prim的下一个点u

        if (u == -1) break; // 所有的点都已连接
        in_mst[u] = true; // 将 u 加入 MST 集合

        // 2. 记录 MST 边（如果你需要可视化）
        if (parent[u] != -1) {
            // 边是 parent[u] 到 u，距离是 key[u]
            mst[parent[u]][u] = key[u];
            //mst[u][parent[u]] = key[u];
            std::cout << "Edge added: " << parent[u] << " - " << u << " with distance " << key[u] << std::endl;
        }

        // 3. 更新 u 的所有邻居 v 的 key 值（**核心扩张步骤**）
        for (int v = 0; v < point_size; v++) {
            // res[u][v] 是 u 到 v 的距离
            if (!in_mst[v] && res[u][v] < key[v]) {
                // 每次直接保留上一个取得的顶点，
                parent[v] = u; // 记录 u 为 v 的父节点
                key[v] = res[u][v]; // 更新最小连接距离
            }
        }
    }
	// MST构建完成
	// 下面，我想把这些最小生成树的边设置为红色，对应回到图中的网格中去
    // 帮我想一下怎么做
    for(int i = 0; i < point_size; i++) {
        for (int j = i + 1; j < point_size; j++) {
            if (mst[i][j] != 0 || mst[j][i] != 0) {
                // i 和 j 之间有一条MST边，将它们的顶点颜色设置为红色
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
    std::vector<bool> visited(size, false);// 访问数组
    std::vector<double> dist(size, INF); //距离初始化为无穷大
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
	return dist;

}



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