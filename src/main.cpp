// #include <iostream>
// #include <ostream>
// #include <source_location>
// #include <corecrt_math.h>
// #include <luisa/luisa-compute.h>
// #include <stb/stb_image_write.h>
// #define TINYOBJLOADER_IMPLEMENTATION
// #include "common/tiny_obj_loader.h"
// #include "common/happly.h"
// // #include "complex.hpp"
// #include "colors.hpp"
//
// using namespace luisa;
// using namespace luisa::compute;

#include "load_pbrt_scene.hpp"
#include <cstddef>
#include <exception>
#include <iostream>
#include <fstream>
#include <filesystem>

int main() {
    std::filesystem::path file_path;
    std::string file_path_str;
    std::cout << "输入文本文件路径: ";
    std::cin >> file_path_str;
    std::cout << "\n";
    try {
        file_path = std::filesystem::canonical(file_path_str);
    } catch (const std::exception& exception) {
        std::cerr << "无法读取文件 " << file_path_str << ": " << exception.what() << "\n";
        return 1;
    }
    if (!std::filesystem::is_regular_file(file_path)) {
        std::cerr << "输入的路径不是文件: " << file_path_str << "\n";
        return 1;
    }
    auto parser = pbrt_parser::base_file_parser::Parser(file_path);

    /*
        for(auto it : obj)
        等同于
        for (auto it = obj.begin(); it != obj.end(); ++it)
    */
    for (auto i : parser) {
        std::cout << i.data << "\n";
    }
}
