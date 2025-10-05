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
    // TODO: 在这里实现你的网格处理算法
    // 示例：简单的顶点位移
    for (auto& vertex : mesh.vertices) {
        // 示例：将每个顶点向某个方向移动一小段距离
        // 你可以替换成你自己的处理逻辑
        vertex->position += Eigen::Vector3d(0.1, 0.1, 0.0);
    }
}