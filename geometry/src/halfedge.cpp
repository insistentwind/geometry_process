#include "halfedge.h"
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <limits>

namespace geometry {

// ============================================================================
// Vertex 类方法实现
// ============================================================================

/**
 * @brief 计算顶点法向量（基于相邻面法向量的加权平均）
 */
void Vertex::computeNormal() {
    if (!halfEdge) return;
    
    normal = Eigen::Vector3d::Zero();
    HalfEdge* he = halfEdge;
    int count = 0;
    
    // 遍历所有相邻的面
    do {
        if (he->face) {
            // 按面积加权
            double area = he->face->computeArea();
            normal += he->face->normal * area;
            count++;
        }
        he = he->pair ? he->pair->next : nullptr;
        if (!he) break; // 边界顶点
    } while (he != halfEdge);
    
    // 标准化
    if (count > 0 && normal.norm() > 0) {
        normal.normalize();
    }
}

/**
 * @brief 获取顶点度数（相邻边的数量）
 */
int Vertex::getDegree() const {
    if (!halfEdge) return 0;
    
    int degree = 0;
    HalfEdge* he = halfEdge;
    
    do {
        degree++;
        he = he->pair ? he->pair->next : nullptr;
        if (!he) break; // 边界顶点
    } while (he != halfEdge);
    
    return degree;
}

/**
 * @brief 检查是否为边界顶点
 */
bool Vertex::isBoundary() const {
    if (!halfEdge) return true;
    
    HalfEdge* he = halfEdge;
    do {
        if (!he->pair) return true; // 发现边界边
        he = he->pair->next;
        if (!he) return true;
    } while (he != halfEdge);
    
    return false;
}

// ============================================================================
// Face 类方法实现
// ============================================================================

/**
 * @brief 计算面法向量（使用Newell方法，适用于任意多边形）
 */
void Face::computeNormal() {
    if (!halfEdge) return;
    
    normal = Eigen::Vector3d::Zero();
    HalfEdge* he = halfEdge;
    
    do {
        Eigen::Vector3d v1 = he->vertex->position;
        Eigen::Vector3d v2 = he->next->vertex->position;
        
        // Newell方法累积法向量
        normal.x() += (v1.y() - v2.y()) * (v1.z() + v2.z());
        normal.y() += (v1.z() - v2.z()) * (v1.x() + v2.x());
        normal.z() += (v1.x() - v2.x()) * (v1.y() + v2.y());
        
        he = he->next;
    } while (he != halfEdge);
    
    // 标准化
    if (normal.norm() > 0) {
        normal.normalize();
    }
}

/**
 * @brief 计算面积（适用于任意凸多边形）
 */
double Face::computeArea() const {
    if (!halfEdge) return 0.0;
    
    double area = 0.0;
    HalfEdge* he = halfEdge;
    Eigen::Vector3d v0 = he->vertex->position;
    
    he = he->next;
    while (he->next != halfEdge) {
        Eigen::Vector3d v1 = he->vertex->position;
        Eigen::Vector3d v2 = he->next->vertex->position;
        
        // 使用三角形面积公式
        Eigen::Vector3d cross = (v1 - v0).cross(v2 - v0);
        area += 0.5 * cross.norm();
        
        he = he->next;
    }
    
    return area;
}

/**
 * @brief 获取面的顶点数
 */
int Face::getVertexCount() const {
    if (!halfEdge) return 0;
    
    int count = 0;
    HalfEdge* he = halfEdge;
    
    do {
        count++;
        he = he->next;
    } while (he != halfEdge);
    
    return count;
}

// ============================================================================
// HalfEdge 类方法实现
// ============================================================================

/**
 * @brief 获取半边的长度
 */
double HalfEdge::getLength() const {
    if (!vertex || !next || !next->vertex) return 0.0;
    return (next->vertex->position - vertex->position).norm();
}

/**
 * @brief 获取半边的方向向量
 */
Eigen::Vector3d HalfEdge::getDirection() const {
    if (!vertex || !next || !next->vertex) {
        return Eigen::Vector3d::Zero();
    }
    Eigen::Vector3d dir = next->vertex->position - vertex->position;
    if (dir.norm() > 0) {
        dir.normalize();
    }
    return dir;
}

// ============================================================================
// HalfEdgeMesh 类方法实现
// ============================================================================

/**
 * @brief 清空半边网格中的所有数据
 */
void HalfEdgeMesh::clear() {
    vertices.clear();
    faces.clear();
    halfEdges.clear();
}

/**
 * @brief 从OBJ格式数据构建半边网格结构（仅位置）
 */
void HalfEdgeMesh::buildFromOBJ(const std::vector<Eigen::Vector3d>& vertexPositions,
                               const std::vector<std::vector<int>>& faceIndices) {
    clear();

    // 输入验证
    if (vertexPositions.empty() || faceIndices.empty()) {
        return;
    }

    // 创建顶点
    vertices.reserve(vertexPositions.size());
    for (size_t i = 0; i < vertexPositions.size(); ++i) {
        vertices.push_back(std::make_unique<Vertex>(vertexPositions[i], static_cast<int>(i)));
    }

    // 用于跟踪已创建的半边
    std::map<std::pair<int, int>, HalfEdge*> edgeMap;

    // 处理每个面
    faces.reserve(faceIndices.size());
    for (size_t faceIdx = 0; faceIdx < faceIndices.size(); ++faceIdx) {
        const auto& faceVerts = faceIndices[faceIdx];
        if (faceVerts.size() < 3) {
            std::cerr << "Warning: Skipping face " << faceIdx << " with less than 3 vertices" << std::endl;
            continue;
        }

        // 验证顶点索引
        bool validFace = true;
        for (int vertexIdx : faceVerts) {
            if (vertexIdx < 0 || vertexIdx >= static_cast<int>(vertices.size())) {
                std::cerr << "Error: Invalid vertex index " << vertexIdx << " in face " << faceIdx << std::endl;
                validFace = false;
                break;
            }
        }
        if (!validFace) continue;

        // 创建新面
        auto face = std::make_unique<Face>(static_cast<int>(faceIdx));
        std::vector<HalfEdge*> faceHalfEdges;
        faceHalfEdges.reserve(faceVerts.size());

        // 为面的每条边创建半边
        for (size_t i = 0; i < faceVerts.size(); ++i) {
            int currentIdx = faceVerts[i];
            int nextIdx = faceVerts[(i + 1) % faceVerts.size()];

            // 创建新半边
            auto halfEdge = std::make_unique<HalfEdge>();
            halfEdge->vertex = vertices[currentIdx].get();
            halfEdge->face = face.get();

            // 先添加到容器中
            halfEdges.push_back(std::move(halfEdge));
            HalfEdge* currentHalfEdge = halfEdges.back().get();

            // 存储半边指针以便后续连接
            faceHalfEdges.push_back(currentHalfEdge);

            // 查找或创建对偶半边
            HalfEdge* pair = getOrCreateHalfEdgePair(
                vertices[currentIdx].get(),
                vertices[nextIdx].get(),
                edgeMap,
                currentHalfEdge
            );

            if (pair) {
                currentHalfEdge->pair = pair;
                pair->pair = currentHalfEdge;
            }

            // 如果这是顶点的第一条出边，设置顶点的半边指针
            if (!vertices[currentIdx]->halfEdge) {
                vertices[currentIdx]->halfEdge = currentHalfEdge;
            }
        }

        // 连接面的半边
        for (size_t i = 0; i < faceHalfEdges.size(); ++i) {
            faceHalfEdges[i]->next = faceHalfEdges[(i + 1) % faceHalfEdges.size()];
            faceHalfEdges[i]->prev = faceHalfEdges[(i + faceHalfEdges.size() - 1) % faceHalfEdges.size()];
        }

        // 设置面的半边指针
        face->halfEdge = faceHalfEdges[0];
        faces.push_back(std::move(face));
    }

    std::cout << "HalfEdgeMesh built successfully: " 
              << vertices.size() << " vertices, " 
              << faces.size() << " faces, " 
              << halfEdges.size() << " half-edges" << std::endl;
    
    // 自动计算法向量
    computeNormals();
}

/**
 * @brief 从完整顶点数据构建半边结构（包含法向量和纹理坐标）
 */
void HalfEdgeMesh::buildFromOBJ(const std::vector<Eigen::Vector3d>& vertexPositions,
                               const std::vector<Eigen::Vector3d>& vertexNormals,
                               const std::vector<Eigen::Vector2d>& vertexTexCoords,
                               const std::vector<std::vector<int>>& faceIndices) {
    clear();

    // 输入验证
    if (vertexPositions.empty() || faceIndices.empty()) {
        return;
    }

    // 创建顶点，包含所有属性
    vertices.reserve(vertexPositions.size());
    for (size_t i = 0; i < vertexPositions.size(); ++i) {
        auto vertex = std::make_unique<Vertex>(vertexPositions[i], static_cast<int>(i));
        
        // 设置法向量（如果提供）
        if (i < vertexNormals.size()) {
            vertex->normal = vertexNormals[i];
        }
        
        // 设置纹理坐标（如果提供）
        if (i < vertexTexCoords.size()) {
            vertex->texCoords = vertexTexCoords[i];
        }
        
        vertices.push_back(std::move(vertex));
    }

    // 用于跟踪已创建的半边
    std::map<std::pair<int, int>, HalfEdge*> edgeMap;

    // 处理每个面
    faces.reserve(faceIndices.size());
    for (size_t faceIdx = 0; faceIdx < faceIndices.size(); ++faceIdx) {
        const auto& faceVerts = faceIndices[faceIdx];
        if (faceVerts.size() < 3) {
            std::cerr << "Warning: Skipping face " << faceIdx << " with less than 3 vertices" << std::endl;
            continue;
        }

        // 验证顶点索引
        bool validFace = true;
        for (int vertexIdx : faceVerts) {
            if (vertexIdx < 0 || vertexIdx >= static_cast<int>(vertices.size())) {
                std::cerr << "Error: Invalid vertex index " << vertexIdx << " in face " << faceIdx << std::endl;
                validFace = false;
                break;
            }
        }
        if (!validFace) continue;

        // 创建新面
        auto face = std::make_unique<Face>(static_cast<int>(faceIdx));
        std::vector<HalfEdge*> faceHalfEdges;
        faceHalfEdges.reserve(faceVerts.size());

        // 为面的每条边创建半边
        for (size_t i = 0; i < faceVerts.size(); ++i) {
            int currentIdx = faceVerts[i];
            int nextIdx = faceVerts[(i + 1) % faceVerts.size()];

            // 创建新半边
            auto halfEdge = std::make_unique<HalfEdge>();
            halfEdge->vertex = vertices[currentIdx].get();
            halfEdge->face = face.get();

            // 先添加到容器中
            halfEdges.push_back(std::move(halfEdge));
            HalfEdge* currentHalfEdge = halfEdges.back().get();

            // 存储半边指针以便后续连接
            faceHalfEdges.push_back(currentHalfEdge);

            // 查找或创建对偶半边
            HalfEdge* pair = getOrCreateHalfEdgePair(
                vertices[currentIdx].get(),
                vertices[nextIdx].get(),
                edgeMap,
                currentHalfEdge
            );

            if (pair) {
                currentHalfEdge->pair = pair;
                pair->pair = currentHalfEdge;
            }

            // 如果这是顶点的第一条出边，设置顶点的半边指针
            if (!vertices[currentIdx]->halfEdge) {
                vertices[currentIdx]->halfEdge = currentHalfEdge;
            }
        }

        // 连接面的半边
        for (size_t i = 0; i < faceHalfEdges.size(); ++i) {
            faceHalfEdges[i]->next = faceHalfEdges[(i + 1) % faceHalfEdges.size()];
            faceHalfEdges[i]->prev = faceHalfEdges[(i + faceHalfEdges.size() - 1) % faceHalfEdges.size()];
        }

        // 设置面的半边指针
        face->halfEdge = faceHalfEdges[0];
        faces.push_back(std::move(face));
    }

    std::cout << "HalfEdgeMesh built successfully with attributes: " 
              << vertices.size() << " vertices, " 
              << faces.size() << " faces, " 
              << halfEdges.size() << " half-edges" << std::endl;
    
    // 如果没有提供法向量，自动计算
    if (vertexNormals.empty()) {
        computeNormals();
    }
}

/**
 * @brief 计算所有顶点和面的法向量
 */
void HalfEdgeMesh::computeNormals() {
    // 先计算所有面的法向量
    for (auto& face : faces) {
        face->computeNormal();
    }
    
    // 再计算所有顶点的法向量（基于相邻面）
    for (auto& vertex : vertices) {
        vertex->computeNormal();
    }
}

/**
 * @brief 获取或创建半边的对偶边
 */
HalfEdge* HalfEdgeMesh::getOrCreateHalfEdgePair(Vertex* v1, Vertex* v2,
                                                std::map<std::pair<int, int>, HalfEdge*>& edgeMap,
                                                HalfEdge* currentHalfEdge) {
    // 查找对偶边 (v2 -> v1)
    auto key = std::make_pair(v2->index, v1->index);
    auto it = edgeMap.find(key);
    if (it != edgeMap.end()) {
        return it->second;
    }

    // 如果没找到对偶边，记录当前边 (v1 -> v2)
    edgeMap[std::make_pair(v1->index, v2->index)] = currentHalfEdge;
    return nullptr;
}

/**
 * @brief 验证半边网格结构的完整性和正确性
 */
bool HalfEdgeMesh::isValid() const {
    // 检查基本结构
    if (vertices.empty() || faces.empty() || halfEdges.empty()) {
        return isEmpty(); // 空网格是有效的
    }

    // 检查每个顶点
    for (const auto& vertex : vertices) {
        if (!vertex || !vertex->halfEdge) {
            std::cerr << "Vertex " << vertex->index << " has no outgoing half-edge" << std::endl;
            return false;
        }
    }

    // 检查每个面
    for (const auto& face : faces) {
        if (!face || !face->halfEdge) {
            std::cerr << "Face " << face->index << " has no half-edge" << std::endl;
            return false;
        }
    }

    // 检查每个半边
    for (const auto& he : halfEdges) {
        if (!he || !he->vertex || !he->face || !he->next || !he->prev) {
            std::cerr << "Half-edge has null pointers" << std::endl;
            return false;
        }

        // 检查环形连接
        if (he->next->prev != he.get()) {
            std::cerr << "Half-edge next/prev connection broken" << std::endl;
            return false;
        }

        // 检查对偶边（如果存在）
        if (he->pair && he->pair->pair != he.get()) {
            std::cerr << "Half-edge pair connection broken" << std::endl;
            return false;
        }
    }

    return true;
}

/**
 * @brief 计算网格的包围盒
 */
std::pair<Eigen::Vector3d, Eigen::Vector3d> HalfEdgeMesh::getBoundingBox() const {
    if (vertices.empty()) {
        return {Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero()};
    }
    
    Eigen::Vector3d minPoint = vertices[0]->position;
    Eigen::Vector3d maxPoint = vertices[0]->position;
    
    for (const auto& vertex : vertices) {
        const auto& pos = vertex->position;
        minPoint = minPoint.cwiseMin(pos);
        maxPoint = maxPoint.cwiseMax(pos);
    }
    
    return {minPoint, maxPoint};
}

/**
 * @brief 获取网格的总表面积
 */
double HalfEdgeMesh::getTotalSurfaceArea() const {
    double totalArea = 0.0;
    for (const auto& face : faces) {
        totalArea += face->computeArea();
    }
    return totalArea;
}

} // namespace geometry