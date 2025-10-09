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

/* 提取顶点颜色 (若未来算法写入顶点颜色) */
std::vector<QVector3D> MeshProcessor::extractColors() const {
    std::vector<QVector3D> cols; 
    cols.reserve(mesh.vertices.size());
    for (const auto& vp : mesh.vertices) {
        if (vp) {
            const auto& c = vp->color;
            cols.emplace_back((float)c.x(), (float)c.y(), (float)c.z());
        }
        else {
            cols.emplace_back(1.f, 1.f, 1.f);
        }
    }
    return cols;
}

//homework2
// 用的是封闭曲面
void MeshProcessor::processGeometry() {
    //meanCurvature();
	cotangentCurvature();
	//gaussianCurvature();
}
//平均曲率实现
void MeshProcessor::meanCurvature() {
    int size = mesh.vertices.size();

    for (int i = 0; i < size; i++) {
        //对于每个顶点，计算它的一阶邻域对应的平均曲率
        geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
        Eigen::Vector3d mean_curvature = { 0 , 0, 0 };
        Eigen::Vector3d total_value = { 0 , 0, 0 };//记录定点数总和
        int count = 0;//记录这个邻域的大小
        do {
            total_value += hf->getEndVertex()->position;
            count++;
            hf = hf->pair->next;// 遍历下一个顶点
        } while (hf != mesh.vertices[i]->halfEdge);
        mean_curvature = mesh.vertices[i]->position * count - total_value;// 颜色对应基本系数
        mesh.vertices[i]->color = mean_curvature * 255;
    }
}
// cotangent曲率实现
void MeshProcessor::cotangentCurvature() {
	int size = mesh.vertices.size();

    std::vector<double> curvature_magnitudes(size);
    double global_max_mag = -1e10;
    double global_min_mag = 1e10;

	//遍历每个顶点，先拿到一阶邻域的所有顶点个数
    for (int i = 0; i < size; i++) {
		if (mesh.vertices[i]->isBoundary()) continue;//跳过边界点
		double area = 0.0;//记录区域面积
        Eigen::Vector3d cotangent_curvature = { 0, 0, 0 };
		geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
        // 从这个点出发，先拿到所有的一阶邻域的顶点
		std::vector<geometry::Vertex*> one_ring_vertices;
        while(hf -> pair -> next != mesh.vertices[i]->halfEdge){
            one_ring_vertices.push_back(hf->getEndVertex());
            hf = hf->pair->next;
		}
		int one_ring_size = one_ring_vertices.size();

        for (int j = 0; j < one_ring_size; j++) {
			//开始计算这个顶点的cotangent曲率
			geometry::Vertex* v0 = one_ring_vertices[(j - 1 + one_ring_size) %one_ring_size];
			geometry::Vertex* v1 = one_ring_vertices[j];
			geometry::Vertex* v2 = one_ring_vertices[(j + 1)%one_ring_size];
			geometry::Vertex* vi = mesh.vertices[i].get();

			Eigen::Vector3d v0v1 = v1->position - v0->position;
			Eigen::Vector3d v0vi = vi->position - v0->position;

			Eigen::Vector3d v2v1 = v1->position - v2->position;
            Eigen::Vector3d v2vi = vi->position - v2->position;

			//double cos_theta0 = v0v1.dot(v0vi) ;
            double cos_theta0 = v0v1.dot(v0vi) / (v0v1.norm() * v0vi.norm());

			//double cos_theta1 = (v2v1).dot(v2vi) ;
            double cos_theta1 = (v2v1).dot(v2vi) / (v2v1.norm() * v2vi.norm());

			double cot_0 = cos_theta0 / sqrt(1 - cos_theta0 * cos_theta0);

            double cot_1 = cos_theta1 / sqrt(1 - cos_theta1 * cos_theta1);

            cotangent_curvature += (cot_0 + cot_1) * (v1->position - vi->position);
            // 计算这两块三角形所占面积
			area += (v0v1.cross(v0vi)).norm() + (v2v1.cross(v2vi)).norm();//按照矩形来算了
        }

		cotangent_curvature = cotangent_curvature / (4 * area);
        // --- 最终归一化，计算模长，并记录 ---
        double curvature_magnitude = cotangent_curvature.norm();

        curvature_magnitudes[i] = curvature_magnitude;

        // 更新全局最大/最小值
        global_max_mag = std::max(global_max_mag, curvature_magnitude);
        global_min_mag = std::min(global_min_mag, curvature_magnitude);

        // 颜色映射: 低曲率 -> 淡蓝, 中等 -> 绿色, 高曲率 -> 红色
        double range = std::max(1e-12, global_max_mag - global_min_mag); // 防止除零
        double lowT  = global_min_mag + range * 0.1; // 低阈值
        double highT = global_min_mag + range * 0.35; // 高阈值
        Eigen::Vector3d color;
        if (curvature_magnitude <= lowT) {
            // 淡蓝 (light blue)
            color = Eigen::Vector3d(0.0, 0.0, 1.0);
        } else if (curvature_magnitude <= highT) {
            // 绿色 (medium)
            color = Eigen::Vector3d(0.0, 1.0, 0.0);
        } else {
            // 红色 (high)
            color = Eigen::Vector3d(1.0, 0.0, 0.0);
        }
        mesh.vertices[i]->color = color * 255.0;
        std::cout << "vertex " << i << " cotangent_curvature: " << cotangent_curvature.transpose() << std::endl;
    }

}

void MeshProcessor::gaussianCurvature() {
    int size = mesh.vertices.size();
    std::vector<double> curvature(size, 0);
    double max_curvature = 1;
    //遍历每个顶点，先拿到一阶邻域的所有顶点个数
    for (int i = 0; i < size; i++) {
        if (mesh.vertices[i]->isBoundary()) continue;//跳过边界点
        
        double gauss_curvature = 0.0;

        double theta = 0.0;
		double area = 0.0;//记录区域面积
        geometry::HalfEdge* hf = mesh.vertices[i]->halfEdge;
        // 从这个点出发，先拿到所有的一阶邻域的顶点
        std::vector<geometry::Vertex*> one_ring_vertices;
        while (hf->pair->next != mesh.vertices[i]->halfEdge) {
            one_ring_vertices.push_back(hf->getEndVertex());
            hf = hf->pair->next;
        }
        int one_ring_size = one_ring_vertices.size();

        for (int j = 0; j < one_ring_size; j++) {
            //开始计算这个顶点的gauss曲率
            geometry::Vertex* v0 = one_ring_vertices[(j - 1 + one_ring_size) % one_ring_size];
            geometry::Vertex* v1 = one_ring_vertices[j];
            geometry::Vertex* vi = mesh.vertices[i].get();

			Eigen::Vector3d viv1 = v1->position - vi->position;
			Eigen::Vector3d viv0 = v0->position - vi->position;

			theta += acos(viv1.dot(viv0) / (viv1.norm() * viv0.norm()));
            //theta += acos(viv1.dot(viv0));

			area += (viv1.cross(viv0)).norm() / 2.0;
			
        }
        gauss_curvature = (2 * M_PI - theta) / area;

        curvature[i] = gauss_curvature;
        max_curvature = std::min(max_curvature, std::abs(gauss_curvature));
		std::cout << "vertex " << i << " gauss_curvature: " << gauss_curvature << std::endl;
		//mesh.vertices[i]->color = Eigen::Vector3d(gauss_curvature, gauss_curvature, gauss_curvature) * 255;
    }
    std::cout << "max gauss_curvature: " << max_curvature << std::endl;

    // 复制并排序，用于查找百分位数
    std::vector<double> sorted_curvatures = curvature;
    std::sort(sorted_curvatures.begin(), sorted_curvatures.end());

    for (int i = 0; i < size; i++) {
		// 直接设置颜色，蓝色表示负曲率，红色表示正曲率
		double K_i = curvature[i];
		Eigen::Vector3d color;
        if (K_i > 0) {
            color = Eigen::Vector3d(1.0, 0.0, 0.0); // 红色分量: 1, 绿色分量: 0, 蓝色分量: 0
        }
        else {
            color = Eigen::Vector3d(0.0, 0.0, 1.0); // 红色分量: 0, 绿色分量: 0, 蓝色分量: 1
		}
		mesh.vertices[i]->color = color * 255.0;
    }

    //// --- 1. 确定鲁棒归一化范围 ---
    //// 使用 95% 和 5% 百分位数来排除最极端的 10% 异常值
    //int idx_95 = (int)(size * 0.95);
    //int idx_05 = (int)(size * 0.05);

    //double K_robust_max = sorted_curvatures[idx_95];
    //double K_robust_min = sorted_curvatures[idx_05];

    //// 鲁棒范围的长度
    //double K_RANGE = K_robust_max - K_robust_min;

    //// --- 2. 颜色映射循环 ---
    //if (K_RANGE < 1e-9) {
    //    // 处理所有曲率都相同（或接近）的情况，避免除零
    //    K_RANGE = 1.0;
    //    K_robust_min = K_robust_max - 1.0;
    //}

    //for (int i = 0; i < size; i++) {
    //    // if (mesh.vertices[i]->isBoundary()) continue; // 假设您在这里跳过边界点

    //    double K_i = curvature[i];

    //    // a) 裁剪/钳制 K_i 到鲁棒范围 [K_robust_min, K_robust_max]
    //    K_i = std::min(K_i, K_robust_max);
    //    K_i = std::max(K_i, K_robust_min);

    //    // b) 归一化到 [0, 1] 范围 (K_norm = 0 是 K_robust_min, K_norm = 1 是 K_robust_max)
    //    double K_norm = (K_i - K_robust_min) / K_RANGE;

    //    // c) 三段式高对比度色谱 (蓝 -> 绿 -> 红)
    //    Eigen::Vector3d color;

    //    // K_norm < 0.5: 蓝到绿 (0 -> 1)
    //    if (K_norm < 0.5) {
    //        double t = K_norm * 2.0; // 范围 [0, 1]
    //        color[0] = 0.0;          // 红色分量: 0
    //        color[1] = t;            // 绿色分量: 0 -> 1
    //        color[2] = 1.0 - t;      // 蓝色分量: 1 -> 0
    //    }
    //    // K_norm >= 0.5: 绿到红 (1 -> 0)
    //    else {
    //        double t = (K_norm - 0.5) * 2.0; // 范围 [0, 1]
    //        color[0] = t;            // 红色分量: 0 -> 1
    //        color[1] = 1.0 - t;      // 绿色分量: 1 -> 0
    //        color[2] = 0.0;          // 蓝色分量: 0
    //    }

    //    // 转换为 [0, 255] 范围
    //    mesh.vertices[i]->color = color * 255.0;
    //}


}



