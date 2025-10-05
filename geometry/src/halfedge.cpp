#include "halfedge.h"
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <limits>

namespace geometry {

// ============================================================================
// Vertex �෽��ʵ��
// ============================================================================

/**
 * @brief ���㶥�㷨���������������淨�����ļ�Ȩƽ����
 */
void Vertex::computeNormal() {
    if (!halfEdge) return;
    
    normal = Eigen::Vector3d::Zero();
    HalfEdge* he = halfEdge;
    int count = 0;
    
    // �����������ڵ���
    do {
        if (he->face) {
            // �������Ȩ
            double area = he->face->computeArea();
            normal += he->face->normal * area;
            count++;
        }
        he = he->pair ? he->pair->next : nullptr;
        if (!he) break; // �߽綥��
    } while (he != halfEdge);
    
    // ��׼��
    if (count > 0 && normal.norm() > 0) {
        normal.normalize();
    }
}

/**
 * @brief ��ȡ������������ڱߵ�������
 */
int Vertex::getDegree() const {
    if (!halfEdge) return 0;
    
    int degree = 0;
    HalfEdge* he = halfEdge;
    
    do {
        degree++;
        he = he->pair ? he->pair->next : nullptr;
        if (!he) break; // �߽綥��
    } while (he != halfEdge);
    
    return degree;
}

/**
 * @brief ����Ƿ�Ϊ�߽綥��
 */
bool Vertex::isBoundary() const {
    if (!halfEdge) return true;
    
    HalfEdge* he = halfEdge;
    do {
        if (!he->pair) return true; // ���ֱ߽��
        he = he->pair->next;
        if (!he) return true;
    } while (he != halfEdge);
    
    return false;
}

// ============================================================================
// Face �෽��ʵ��
// ============================================================================

/**
 * @brief �����淨������ʹ��Newell�������������������Σ�
 */
void Face::computeNormal() {
    if (!halfEdge) return;
    
    normal = Eigen::Vector3d::Zero();
    HalfEdge* he = halfEdge;
    
    do {
        Eigen::Vector3d v1 = he->vertex->position;
        Eigen::Vector3d v2 = he->next->vertex->position;
        
        // Newell�����ۻ�������
        normal.x() += (v1.y() - v2.y()) * (v1.z() + v2.z());
        normal.y() += (v1.z() - v2.z()) * (v1.x() + v2.x());
        normal.z() += (v1.x() - v2.x()) * (v1.y() + v2.y());
        
        he = he->next;
    } while (he != halfEdge);
    
    // ��׼��
    if (normal.norm() > 0) {
        normal.normalize();
    }
}

/**
 * @brief �������������������͹����Σ�
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
        
        // ʹ�������������ʽ
        Eigen::Vector3d cross = (v1 - v0).cross(v2 - v0);
        area += 0.5 * cross.norm();
        
        he = he->next;
    }
    
    return area;
}

/**
 * @brief ��ȡ��Ķ�����
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
// HalfEdge �෽��ʵ��
// ============================================================================

/**
 * @brief ��ȡ��ߵĳ���
 */
double HalfEdge::getLength() const {
    if (!vertex || !next || !next->vertex) return 0.0;
    return (next->vertex->position - vertex->position).norm();
}

/**
 * @brief ��ȡ��ߵķ�������
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
// HalfEdgeMesh �෽��ʵ��
// ============================================================================

/**
 * @brief ��հ�������е���������
 */
void HalfEdgeMesh::clear() {
    vertices.clear();
    faces.clear();
    halfEdges.clear();
}

/**
 * @brief ��OBJ��ʽ���ݹ����������ṹ����λ�ã�
 */
void HalfEdgeMesh::buildFromOBJ(const std::vector<Eigen::Vector3d>& vertexPositions,
                               const std::vector<std::vector<int>>& faceIndices) {
    clear();

    // ������֤
    if (vertexPositions.empty() || faceIndices.empty()) {
        return;
    }

    // ��������
    vertices.reserve(vertexPositions.size());
    for (size_t i = 0; i < vertexPositions.size(); ++i) {
        vertices.push_back(std::make_unique<Vertex>(vertexPositions[i], static_cast<int>(i)));
    }

    // ���ڸ����Ѵ����İ��
    std::map<std::pair<int, int>, HalfEdge*> edgeMap;

    // ����ÿ����
    faces.reserve(faceIndices.size());
    for (size_t faceIdx = 0; faceIdx < faceIndices.size(); ++faceIdx) {
        const auto& faceVerts = faceIndices[faceIdx];
        if (faceVerts.size() < 3) {
            std::cerr << "Warning: Skipping face " << faceIdx << " with less than 3 vertices" << std::endl;
            continue;
        }

        // ��֤��������
        bool validFace = true;
        for (int vertexIdx : faceVerts) {
            if (vertexIdx < 0 || vertexIdx >= static_cast<int>(vertices.size())) {
                std::cerr << "Error: Invalid vertex index " << vertexIdx << " in face " << faceIdx << std::endl;
                validFace = false;
                break;
            }
        }
        if (!validFace) continue;

        // ��������
        auto face = std::make_unique<Face>(static_cast<int>(faceIdx));
        std::vector<HalfEdge*> faceHalfEdges;
        faceHalfEdges.reserve(faceVerts.size());

        // Ϊ���ÿ���ߴ������
        for (size_t i = 0; i < faceVerts.size(); ++i) {
            int currentIdx = faceVerts[i];
            int nextIdx = faceVerts[(i + 1) % faceVerts.size()];

            // �����°��
            auto halfEdge = std::make_unique<HalfEdge>();
            halfEdge->vertex = vertices[currentIdx].get();
            halfEdge->face = face.get();

            // ����ӵ�������
            halfEdges.push_back(std::move(halfEdge));
            HalfEdge* currentHalfEdge = halfEdges.back().get();

            // �洢���ָ���Ա��������
            faceHalfEdges.push_back(currentHalfEdge);

            // ���һ򴴽���ż���
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

            // ������Ƕ���ĵ�һ�����ߣ����ö���İ��ָ��
            if (!vertices[currentIdx]->halfEdge) {
                vertices[currentIdx]->halfEdge = currentHalfEdge;
            }
        }

        // ������İ��
        for (size_t i = 0; i < faceHalfEdges.size(); ++i) {
            faceHalfEdges[i]->next = faceHalfEdges[(i + 1) % faceHalfEdges.size()];
            faceHalfEdges[i]->prev = faceHalfEdges[(i + faceHalfEdges.size() - 1) % faceHalfEdges.size()];
        }

        // ������İ��ָ��
        face->halfEdge = faceHalfEdges[0];
        faces.push_back(std::move(face));
    }

    std::cout << "HalfEdgeMesh built successfully: " 
              << vertices.size() << " vertices, " 
              << faces.size() << " faces, " 
              << halfEdges.size() << " half-edges" << std::endl;
    
    // �Զ����㷨����
    computeNormals();
}

/**
 * @brief �������������ݹ�����߽ṹ���������������������꣩
 */
void HalfEdgeMesh::buildFromOBJ(const std::vector<Eigen::Vector3d>& vertexPositions,
                               const std::vector<Eigen::Vector3d>& vertexNormals,
                               const std::vector<Eigen::Vector2d>& vertexTexCoords,
                               const std::vector<std::vector<int>>& faceIndices) {
    clear();

    // ������֤
    if (vertexPositions.empty() || faceIndices.empty()) {
        return;
    }

    // �������㣬������������
    vertices.reserve(vertexPositions.size());
    for (size_t i = 0; i < vertexPositions.size(); ++i) {
        auto vertex = std::make_unique<Vertex>(vertexPositions[i], static_cast<int>(i));
        
        // ���÷�����������ṩ��
        if (i < vertexNormals.size()) {
            vertex->normal = vertexNormals[i];
        }
        
        // �����������꣨����ṩ��
        if (i < vertexTexCoords.size()) {
            vertex->texCoords = vertexTexCoords[i];
        }
        
        vertices.push_back(std::move(vertex));
    }

    // ���ڸ����Ѵ����İ��
    std::map<std::pair<int, int>, HalfEdge*> edgeMap;

    // ����ÿ����
    faces.reserve(faceIndices.size());
    for (size_t faceIdx = 0; faceIdx < faceIndices.size(); ++faceIdx) {
        const auto& faceVerts = faceIndices[faceIdx];
        if (faceVerts.size() < 3) {
            std::cerr << "Warning: Skipping face " << faceIdx << " with less than 3 vertices" << std::endl;
            continue;
        }

        // ��֤��������
        bool validFace = true;
        for (int vertexIdx : faceVerts) {
            if (vertexIdx < 0 || vertexIdx >= static_cast<int>(vertices.size())) {
                std::cerr << "Error: Invalid vertex index " << vertexIdx << " in face " << faceIdx << std::endl;
                validFace = false;
                break;
            }
        }
        if (!validFace) continue;

        // ��������
        auto face = std::make_unique<Face>(static_cast<int>(faceIdx));
        std::vector<HalfEdge*> faceHalfEdges;
        faceHalfEdges.reserve(faceVerts.size());

        // Ϊ���ÿ���ߴ������
        for (size_t i = 0; i < faceVerts.size(); ++i) {
            int currentIdx = faceVerts[i];
            int nextIdx = faceVerts[(i + 1) % faceVerts.size()];

            // �����°��
            auto halfEdge = std::make_unique<HalfEdge>();
            halfEdge->vertex = vertices[currentIdx].get();
            halfEdge->face = face.get();

            // ����ӵ�������
            halfEdges.push_back(std::move(halfEdge));
            HalfEdge* currentHalfEdge = halfEdges.back().get();

            // �洢���ָ���Ա��������
            faceHalfEdges.push_back(currentHalfEdge);

            // ���һ򴴽���ż���
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

            // ������Ƕ���ĵ�һ�����ߣ����ö���İ��ָ��
            if (!vertices[currentIdx]->halfEdge) {
                vertices[currentIdx]->halfEdge = currentHalfEdge;
            }
        }

        // ������İ��
        for (size_t i = 0; i < faceHalfEdges.size(); ++i) {
            faceHalfEdges[i]->next = faceHalfEdges[(i + 1) % faceHalfEdges.size()];
            faceHalfEdges[i]->prev = faceHalfEdges[(i + faceHalfEdges.size() - 1) % faceHalfEdges.size()];
        }

        // ������İ��ָ��
        face->halfEdge = faceHalfEdges[0];
        faces.push_back(std::move(face));
    }

    std::cout << "HalfEdgeMesh built successfully with attributes: " 
              << vertices.size() << " vertices, " 
              << faces.size() << " faces, " 
              << halfEdges.size() << " half-edges" << std::endl;
    
    // ���û���ṩ���������Զ�����
    if (vertexNormals.empty()) {
        computeNormals();
    }
}

/**
 * @brief �������ж������ķ�����
 */
void HalfEdgeMesh::computeNormals() {
    // �ȼ���������ķ�����
    for (auto& face : faces) {
        face->computeNormal();
    }
    
    // �ټ������ж���ķ����������������棩
    for (auto& vertex : vertices) {
        vertex->computeNormal();
    }
}

/**
 * @brief ��ȡ�򴴽���ߵĶ�ż��
 */
HalfEdge* HalfEdgeMesh::getOrCreateHalfEdgePair(Vertex* v1, Vertex* v2,
                                                std::map<std::pair<int, int>, HalfEdge*>& edgeMap,
                                                HalfEdge* currentHalfEdge) {
    // ���Ҷ�ż�� (v2 -> v1)
    auto key = std::make_pair(v2->index, v1->index);
    auto it = edgeMap.find(key);
    if (it != edgeMap.end()) {
        return it->second;
    }

    // ���û�ҵ���ż�ߣ���¼��ǰ�� (v1 -> v2)
    edgeMap[std::make_pair(v1->index, v2->index)] = currentHalfEdge;
    return nullptr;
}

/**
 * @brief ��֤�������ṹ�������Ժ���ȷ��
 */
bool HalfEdgeMesh::isValid() const {
    // �������ṹ
    if (vertices.empty() || faces.empty() || halfEdges.empty()) {
        return isEmpty(); // ����������Ч��
    }

    // ���ÿ������
    for (const auto& vertex : vertices) {
        if (!vertex || !vertex->halfEdge) {
            std::cerr << "Vertex " << vertex->index << " has no outgoing half-edge" << std::endl;
            return false;
        }
    }

    // ���ÿ����
    for (const auto& face : faces) {
        if (!face || !face->halfEdge) {
            std::cerr << "Face " << face->index << " has no half-edge" << std::endl;
            return false;
        }
    }

    // ���ÿ�����
    for (const auto& he : halfEdges) {
        if (!he || !he->vertex || !he->face || !he->next || !he->prev) {
            std::cerr << "Half-edge has null pointers" << std::endl;
            return false;
        }

        // ��黷������
        if (he->next->prev != he.get()) {
            std::cerr << "Half-edge next/prev connection broken" << std::endl;
            return false;
        }

        // ����ż�ߣ�������ڣ�
        if (he->pair && he->pair->pair != he.get()) {
            std::cerr << "Half-edge pair connection broken" << std::endl;
            return false;
        }
    }

    return true;
}

/**
 * @brief ��������İ�Χ��
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
 * @brief ��ȡ������ܱ����
 */
double HalfEdgeMesh::getTotalSurfaceArea() const {
    double totalArea = 0.0;
    for (const auto& face : faces) {
        totalArea += face->computeArea();
    }
    return totalArea;
}

} // namespace geometry