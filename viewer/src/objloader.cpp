#include "objloader.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool ObjLoader::loadOBJ(const std::string& path)
{
    vertices.clear();
    normals.clear();
    indices.clear();

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open file: " << path << std::endl;
        return false;
    }

    std::vector<QVector3D> temp_vertices;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string lineHeader;
        iss >> lineHeader;

        if (lineHeader == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            temp_vertices.push_back(QVector3D(x, y, z));
        }
        else if (lineHeader == "f") {
            std::string v1, v2, v3;
            iss >> v1 >> v2 >> v3;

            // 提取顶点索引
            unsigned int vertexIndex[3];
            vertexIndex[0] = std::stoi(v1) - 1;
            vertexIndex[1] = std::stoi(v2) - 1;
            vertexIndex[2] = std::stoi(v3) - 1;

            indices.push_back(vertexIndex[0]);
            indices.push_back(vertexIndex[1]);
            indices.push_back(vertexIndex[2]);
        }
    }

    vertices = temp_vertices;

    std::cout << "Loaded " << vertices.size() << " vertices and "
        << indices.size() << " indices" << std::endl;

    return !vertices.empty() && !indices.empty();
}

QString filePath = "E:/qt_work/myWork/bunny.obj";