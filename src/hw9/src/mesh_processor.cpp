#include "mesh_processor.h"
#include <mesh_converter.h>
#include <iostream>
#include <unordered_map>

// OpenMesh decimation
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>

std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
MeshProcessor::processOBJData(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices) {
    
    // 步骤1：使用geometry模块的MeshConverter构建半边网格
    geometry::MeshConverter::buildMeshFromQtData(mesh, vertices, indices);

    // 步骤2：验证半边结构的正确性
    if (!mesh.isValid()) {
        std::cerr << "Warning: Generated half-edge mesh is invalid!" << std::endl;
    }

    int n = 100;
    // 步骤3：执行实际的几何处理操作（这是需要自己实现的部分）
    processGeometry(n);

    // 步骤4：使用geometry模块的MeshConverter将结果转回Qt格式
    return geometry::MeshConverter::convertMeshToQtData(mesh);
}

// QEM网格简化
void MeshProcessor::processGeometry(int n) {
    if (mesh.faces.empty() || mesh.vertices.empty()) return;

    // OpenMesh 的 collapse/decimate 只能作用于 OpenMesh 自己的网格类型，
    // 所以这里做一次: geometry::HalfEdgeMesh -> OpenMesh::TriMesh -> decimate -> 重新构建 HalfEdgeMesh。

    // 1) geometry::HalfEdgeMesh -> OpenMesh::TriMesh
    TriMesh om;
    // Decimater 需要 status 属性来标记删除/折叠等操作
    om.request_vertex_status();
    om.request_edge_status();
    om.request_halfedge_status();
    om.request_face_status();

    // 顶点导入
    std::vector<TriMesh::VertexHandle> vhandles;
    vhandles.reserve(mesh.vertices.size());
    for (const auto& v : mesh.vertices) {
        vhandles.push_back(om.add_vertex(TriMesh::Point(
            static_cast<float>(v->position.x()),
            static_cast<float>(v->position.y()),
            static_cast<float>(v->position.z()))));
    }

    // 面导入（当前 MeshConverter 路径里默认是三角面，这里只接收三角形）
    for (const auto& f : mesh.faces) {
        if (!f || !f->halfEdge) continue;

        std::vector<TriMesh::VertexHandle> face_vh;
        face_vh.reserve(4);
        geometry::HalfEdge* he = f->halfEdge;
        do {
            int idx = he->vertex ? he->vertex->index : -1;
            if (idx >= 0 && static_cast<size_t>(idx) < vhandles.size()) {
                face_vh.push_back(vhandles[idx]);
            }
            he = he->next;
        } while (he && he != f->halfEdge);

        if (face_vh.size() == 3) {
            om.add_face(face_vh);
        }
    }

    if (om.n_faces() == 0) return;

    // 2) OpenMesh QEM decimation
    // 使用 OpenMesh 自带的 Quadric Error Metrics 模块，内部会做合法的 edge collapse。
    using Decimater = OpenMesh::Decimater::DecimaterT<TriMesh>;
    using HModQuadric = OpenMesh::Decimater::ModQuadricT<TriMesh>::Handle;
    Decimater decimater(om);
    HModQuadric hMod;
    decimater.add(hMod);
    decimater.initialize();

    // 这里把 n 解释为「目标面数」(target face count)
    size_t target_faces = (n < 0) ? 0u : static_cast<size_t>(n);
    if (target_faces < 1) target_faces = 1;
    if (target_faces >= om.n_faces()) return;

    decimater.decimate_to_faces(target_faces);
    // 清理被标记删除的元素，重新压紧索引
    om.garbage_collection();

    // 3) OpenMesh::TriMesh -> geometry::HalfEdgeMesh
    // 通过 (vertices, indices) 的形式导出结果，再用 MeshConverter 重新构建半边结构。
    std::vector<QVector3D> outVerts;
    outVerts.reserve(om.n_vertices());

    // OpenMesh 的 voh/fvh 迭代拿到的是 VertexHandle，这里把 handle 的 idx 映射成新的连续 [0..N) 索引
    std::unordered_map<int, int> vhToOutIndex;
    vhToOutIndex.reserve(om.n_vertices());
    for (auto v_it = om.vertices_begin(); v_it != om.vertices_end(); ++v_it) {
        const auto p = om.point(*v_it);
        int newIdx = static_cast<int>(outVerts.size());
        outVerts.emplace_back(p[0], p[1], p[2]);
        vhToOutIndex.emplace(v_it->idx(), newIdx);
    }

    std::vector<unsigned int> outIndices;
    outIndices.reserve(om.n_faces() * 3);
    for (auto f_it = om.faces_begin(); f_it != om.faces_end(); ++f_it) {
        std::vector<unsigned int> tri;
        tri.reserve(3);
        for (auto fv_it = om.fv_begin(*f_it); fv_it.is_valid(); ++fv_it) {
            auto it = vhToOutIndex.find(fv_it->idx());
            if (it != vhToOutIndex.end()) tri.push_back(static_cast<unsigned int>(it->second));
        }
        if (tri.size() == 3) {
            outIndices.push_back(tri[0]);
            outIndices.push_back(tri[1]);
            outIndices.push_back(tri[2]);
        }
    }

    geometry::MeshConverter::buildMeshFromQtData(mesh, outVerts, outIndices);
}