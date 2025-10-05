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

    // 步骤3：执行实际的几何处理操作（这是需要自己实现的部分）
    processGeometry();

    // 步骤4：使用geometry模块的MeshConverter将结果转回Qt格式
    return geometry::MeshConverter::convertMeshToQtData(mesh);
}

void MeshProcessor::processGeometry() {
    // Laplace平滑算法 - 更激进的设置
    // 参数设置
    const int iterations = 20;       // 增加迭代次数到20次
    const double lambda = 0.9;       // 增大平滑系数到0.9（接近1会更平滑）
    
    std::cout << "==== Laplace Smoothing Started ====" << std::endl;
    std::cout << "Vertices: " << mesh.vertices.size() << std::endl;
    std::cout << "Faces: " << mesh.faces.size() << std::endl;
    std::cout << "Iterations: " << iterations << ", Lambda: " << lambda << std::endl;
    std::cout << "WARNING: Using aggressive smoothing parameters!" << std::endl;
    
    for (int iter = 0; iter < iterations; ++iter) {
        // 存储每个顶点的新位置
        std::vector<Eigen::Vector3d> newPositions(mesh.vertices.size());
        int verticesWithNeighbors = 0;
        int totalNeighbors = 0;
        
        // 对每个顶点计算其邻域顶点的平均位置
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            auto& vertex = mesh.vertices[i];
            
            // 收集邻域顶点（一环邻域）
            std::vector<Eigen::Vector3d> neighborPositions;
            
            // 通过半边遍历邻域顶点
            if (vertex->halfEdge) {
                auto startHE = vertex->halfEdge;
                auto currentHE = startHE;
                int maxIterations = 100; // 防止无限循环
                int iterCount = 0;
                
                do {
                    // 当前半边的终点是邻域顶点
                    if (currentHE->next && currentHE->next->vertex) {
                        neighborPositions.push_back(currentHE->next->vertex->position);
                    }
                    
                    // 移动到下一个围绕该顶点的半边
                    // 通过pair半边的next来继续绕顶点旋转
                    if (currentHE->pair && currentHE->pair->next) {
                        currentHE = currentHE->pair->next;
                    } else {
                        // 边界情况：没有pair，说明是边界边
                        break;
                    }
                    
                    iterCount++;
                    if (iterCount >= maxIterations) {
                        std::cerr << "Warning: Potential infinite loop at vertex " << i << std::endl;
                        break;
                    }
                    
                } while (currentHE && currentHE != startHE);
                
                if (!neighborPositions.empty()) {
                    verticesWithNeighbors++;
                    totalNeighbors += neighborPositions.size();
                }
            }
            
            // 计算邻域顶点的平均位置（Laplace算子）
            if (!neighborPositions.empty()) {
                Eigen::Vector3d laplacian(0.0, 0.0, 0.0);
                for (const auto& pos : neighborPositions) {
                    laplacian += pos;
                }
                laplacian /= static_cast<double>(neighborPositions.size());
                
                // 使用加权平均更新位置
                // new_pos = old_pos + lambda * (laplacian - old_pos)
                newPositions[i] = vertex->position + lambda * (laplacian - vertex->position);
            } else {
                // 如果没有邻域（孤立顶点或边界），保持原位置
                newPositions[i] = vertex->position;
            }
        }
        
        // 计算位移量统计
        double totalDisplacement = 0.0;
        double maxDisplacement = 0.0;
        
        // 更新所有顶点位置
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            double displacement = (newPositions[i] - mesh.vertices[i]->position).norm();
            totalDisplacement += displacement;
            maxDisplacement = std::max(maxDisplacement, displacement);
            
            mesh.vertices[i]->position = newPositions[i];
        }
        
        double avgDisplacement = totalDisplacement / mesh.vertices.size();
        double avgNeighbors = verticesWithNeighbors > 0 ? 
            static_cast<double>(totalNeighbors) / verticesWithNeighbors : 0.0;
        
        std::cout << "Iteration " << (iter + 1) << ": "
                  << "Avg displacement = " << avgDisplacement
                  << ", Max displacement = " << maxDisplacement << std::endl;
        
        // 每5次迭代输出一次详细信息
        if ((iter + 1) % 5 == 0) {
            std::cout << "  -> Progress: " << ((iter + 1) * 100 / iterations) << "% "
                      << "(Vertices with neighbors = " << verticesWithNeighbors
                      << ", Avg neighbors = " << avgNeighbors << ")" << std::endl;
        }
    }
    
    std::cout << "==== Laplace Smoothing Completed ====" << std::endl;
}