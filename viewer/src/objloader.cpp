#include "objloader.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool ObjLoader::loadOBJ(const std::string& path) {
    // ��վ�����
    vertices.clear();
    normals.clear();
    indices.clear();

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open file: " << path << std::endl;
        return false;
    }

    std::vector<QVector3D> tempVertices;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string tag;
        iss >> tag;

        if (tag == "v") {
            float x, y, z; iss >> x >> y >> z;
            tempVertices.emplace_back(x, y, z);
        } else if (tag == "f") {
            // ��֧�� "f v1 v2 v3" ��ʽ������������/����(index/index/index)
            std::string a, b, c; iss >> a >> b >> c;
			
            unsigned int vi[3];
			// stoi�������ǽ��ַ���ת��Ϊ����
			vi[0] = static_cast<unsigned int>(std::stoi(a)) - 1;// OBJ ������ 1 ��ʼ
            vi[1] = static_cast<unsigned int>(std::stoi(b)) - 1;
            vi[2] = static_cast<unsigned int>(std::stoi(c)) - 1;
            indices.push_back(vi[0]);
            indices.push_back(vi[1]);
            indices.push_back(vi[2]);
        }
    }

    vertices = std::move(tempVertices);

    std::cout << "Loaded " << vertices.size() << " vertices and "
              << indices.size() << " indices" << std::endl;

    return !vertices.empty() && !indices.empty();
}