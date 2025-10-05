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

    // ����3��ִ��ʵ�ʵļ��δ��������������Ҫ�Լ�ʵ�ֵĲ��֣�
    processGeometry();

    // ����4��ʹ��geometryģ���MeshConverter�����ת��Qt��ʽ
    return geometry::MeshConverter::convertMeshToQtData(mesh);
}

void MeshProcessor::processGeometry() {
    // Laplaceƽ���㷨 - ������������
    // ��������
    const int iterations = 20;       // ���ӵ���������20��
    const double lambda = 0.9;       // ����ƽ��ϵ����0.9���ӽ�1���ƽ����
    
    std::cout << "==== Laplace Smoothing Started ====" << std::endl;
    std::cout << "Vertices: " << mesh.vertices.size() << std::endl;
    std::cout << "Faces: " << mesh.faces.size() << std::endl;
    std::cout << "Iterations: " << iterations << ", Lambda: " << lambda << std::endl;
    std::cout << "WARNING: Using aggressive smoothing parameters!" << std::endl;
    
    for (int iter = 0; iter < iterations; ++iter) {
        // �洢ÿ���������λ��
        std::vector<Eigen::Vector3d> newPositions(mesh.vertices.size());
        int verticesWithNeighbors = 0;
        int totalNeighbors = 0;
        
        // ��ÿ��������������򶥵��ƽ��λ��
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            auto& vertex = mesh.vertices[i];
            
            // �ռ����򶥵㣨һ������
            std::vector<Eigen::Vector3d> neighborPositions;
            
            // ͨ����߱������򶥵�
            if (vertex->halfEdge) {
                auto startHE = vertex->halfEdge;
                auto currentHE = startHE;
                int maxIterations = 100; // ��ֹ����ѭ��
                int iterCount = 0;
                
                do {
                    // ��ǰ��ߵ��յ������򶥵�
                    if (currentHE->next && currentHE->next->vertex) {
                        neighborPositions.push_back(currentHE->next->vertex->position);
                    }
                    
                    // �ƶ�����һ��Χ�Ƹö���İ��
                    // ͨ��pair��ߵ�next�������ƶ�����ת
                    if (currentHE->pair && currentHE->pair->next) {
                        currentHE = currentHE->pair->next;
                    } else {
                        // �߽������û��pair��˵���Ǳ߽��
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
            
            // �������򶥵��ƽ��λ�ã�Laplace���ӣ�
            if (!neighborPositions.empty()) {
                Eigen::Vector3d laplacian(0.0, 0.0, 0.0);
                for (const auto& pos : neighborPositions) {
                    laplacian += pos;
                }
                laplacian /= static_cast<double>(neighborPositions.size());
                
                // ʹ�ü�Ȩƽ������λ��
                // new_pos = old_pos + lambda * (laplacian - old_pos)
                newPositions[i] = vertex->position + lambda * (laplacian - vertex->position);
            } else {
                // ���û�����򣨹��������߽磩������ԭλ��
                newPositions[i] = vertex->position;
            }
        }
        
        // ����λ����ͳ��
        double totalDisplacement = 0.0;
        double maxDisplacement = 0.0;
        
        // �������ж���λ��
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
        
        // ÿ5�ε������һ����ϸ��Ϣ
        if ((iter + 1) % 5 == 0) {
            std::cout << "  -> Progress: " << ((iter + 1) * 100 / iterations) << "% "
                      << "(Vertices with neighbors = " << verticesWithNeighbors
                      << ", Avg neighbors = " << avgNeighbors << ")" << std::endl;
        }
    }
    
    std::cout << "==== Laplace Smoothing Completed ====" << std::endl;
}