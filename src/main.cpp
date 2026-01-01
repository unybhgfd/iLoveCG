#include <iostream>
#include <ostream>
#include <source_location>
#include <corecrt_math.h>
#include <luisa/luisa-compute.h>
#include <stb/stb_image_write.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include "common/tiny_obj_loader.h"
#include "common/happly.h"
// #include "complex.hpp"
// #include "colors.hpp"

using namespace luisa;
using namespace luisa::compute;

int main() {
    std::string input_ply_path;
    std::cout << "输入.ply文件路径: ";
    std::cin >> input_ply_path;
    std::cout << "\n";
    std::filesystem::path ply_path;
    try {
        ply_path = std::filesystem::canonical(input_ply_path);
    } catch (const std::exception& exception) {
        std::cout << "路径 " << input_ply_path << " 不存在: " << exception.what() << "\n";
        return 1;
    }
    happly::PLYData ply_data(ply_path.string());
    auto vertices = ply_data.getVertexPositions();
    auto faces =
        ply_data.getFaceIndices();
    try {
        ply_data.getVertexColors();
    } catch (...) {
        std::cout << "没有给出顶点颜色\n";
    }
}
