#include <iostream>
#include <luisa/luisa-compute.h>
#include <random>
#include <stb/stb_image_write.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include "common/tiny_obj_loader.h"
#include "complex.hpp"
#include "ga.hpp"

using namespace luisa;
using namespace luisa::compute;

struct Obj {
    Accel& accel; // 求交结构
    BindlessArray& heap; // 缓冲区

    Obj(Accel&& accel_var, BindlessArray&& heap_var)
        : accel(accel_var), heap(heap_var) {}
};

Obj loadObj(const std::string& obj_string, Device& device, Stream& stream) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;
    reader_config.vertex_color = false;

    tinyobj::ObjReader reader;
    if (!reader.ParseFromString(obj_string, "", reader_config)) {
        luisa::string_view error_message = "unknown error.";
        if (const auto& err = reader.Error(); !err.empty()) { error_message = err; }
        LUISA_ERROR_WITH_LOCATION("Failed to load OBJ file: {}", error_message);
    }

    const auto& vert = reader.GetAttrib().GetVertices();
    vector<float3> vertices;

    vertices.reserve(vert.size() / 3u);
    for (uint i = 0u; i < vert.size(); i += 3u) {
        vertices.emplace_back(make_float3(
            vert[i + 0u], vert[i + 1u], vert[i + 2u]));
    }
    LUISA_INFO(
        "Loaded mesh with {} shape(s) and {} vertices.",
        reader.GetShapes().size(), vertices.size());


    BindlessArray heap = device.create_bindless_array(65535);
    Buffer<float3> vertex_buffer = device.create_buffer<float3>(vertices.size());
    stream << vertex_buffer.copy_from(vertices.data());
    luisa::vector<Mesh> meshes;
    luisa::vector<Buffer<Triangle>> triangle_buffers;
    for (auto &&shape : reader.GetShapes()) {
        uint index = static_cast<uint>(meshes.size());
        std::vector<tinyobj::index_t> const &mesh_indices = shape.mesh.indices;
        uint triangle_count = mesh_indices.size() / 3u;
        LUISA_INFO(
            "Processing shape '{}' at index {} with {} triangle(s).",
            shape.name, index, triangle_count);
        luisa::vector<uint> indices;
        indices.reserve(mesh_indices.size());
        for (tinyobj::index_t idx : mesh_indices) { indices.emplace_back(idx.vertex_index); }
        Buffer<Triangle> &triangle_buffer = triangle_buffers.emplace_back(device.create_buffer<Triangle>(triangle_count));
        Mesh &mesh = meshes.emplace_back(device.create_mesh(vertex_buffer, triangle_buffer));
        heap.emplace_on_update(index, triangle_buffer);
        stream << triangle_buffer.copy_from(indices.data())
               << mesh.build();
    }

    Accel accel = device.create_accel({});
    for (Mesh &mesh : meshes) {
        accel.emplace_back(mesh, make_float4x4(1.0f));
    }

    stream << heap.update()
           << accel.build()
           << synchronize();

    return Obj{std::move(accel), std::move(heap)};
}

void test_geo_alg() {
    const vga6::ga_type rotor = vga6::random_rotor();
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        LUISA_INFO("Usage: {} <backend>. <backend>: cuda, dx, cpu, metal", argv[0]);
        LUISA_INFO("未输入后端名称， 开始运行测试");
        test_geo_alg();
        exit(1);
    }

    Context context{argv[0]};
    Device device = context.create_device(argv[1]);
    Stream stream = device.create_stream(StreamTag::GRAPHICS);
    // Obj scene = loadObj(obj_string, device, stream);

    Kernel2D main_kernel = [&](
        ImageFloat image,
        Float6x6 transform_mat,
        Float6 translate_vec,
        UInt max_iterations
    ) {
        /*

        */
        set_block_size(16, 16);

        UInt2 img_index = dispatch_id().xy(); // 像素坐标
        Float2 uv_pos = (make_float2(img_index) + 0.5f) / make_float2(dispatch_size().xy()); // uv坐标

        // 向量(u, v, 0, 0, 0, 0)先乘矩阵然后移动
        Float6 pos = transform_mat * def<float6>(
            make_float3(uv_pos - make_float2(0.5, 0.5), 0.F),
            make_float3(0)
        ) + translate_vec;

        /*
        mandelbrot集扩展, 是六维结构
        迭代公式: Z_{n+1} = (Z_n)^X+C
        其中Z_n, X, C都是复数
        */
        Complex mb_z{pos.first.x, pos.first.y};
        Complex original_z = mb_z;
        Complex mb_x{pos.first.z, pos.second.x};
        Complex mb_c{pos.second.y, pos.second.z};

        // 迭代
        UInt iterations_cnt = 0;
        for (auto iterate_idx: dynamic_range(max_iterations)) {
            iterations_cnt += 1;
            if_((mb_z - original_z)->abs_square() > 1000, break_);

            mb_z = mb_z->pow(mb_x) + mb_c;
        };

        Float grey_level;
        Float4 color;
        if_(iterations_cnt == max_iterations, [&] {
            color = make_float4(uv_pos, 1.F, 1.F);
        }).else_([&] {
            grey_level = iterations_cnt.cast<float>() / max_iterations;
            color = make_float4(make_float3(grey_level), 1.F);
        });
        image.write(img_index, color);
    };
    Shader main_shader = device.compile(main_kernel);

    // RNG
    std::random_device seed;
    std::mt19937 engine(seed());
    std::uniform_real_distribution<float> distribution(-1, .1);

    // 初始设置
    constexpr size_t image_width =  1024;
    constexpr size_t image_height = 1024;
    constexpr uint render_times = 1000;
    const filesystem::path file_save_path = filesystem::current_path() / "output";
    const vga6::ga_type rotor_start = vga6::random_rotor();
    const vga6::ga_type rotor_end = vga6::random_rotor();
    constexpr float mat_coeff = 10.; // 旋转矩阵的系数
    float6 translate_vec = make_float6(
        #define R distribution(engine) * 2.
        R, R, R, R, R, R
        #undef R
    );
    const std::array<vga6::ga_type, 6> ga_basis{
        vga6::make_ga_point({1, 0, 0, 0, 0, 0}),
        vga6::make_ga_point({0, 1, 0, 0, 0, 0}),
        vga6::make_ga_point({0, 0, 1, 0, 0, 0}),
        vga6::make_ga_point({0, 0, 0, 1, 0, 0}),
        vga6::make_ga_point({0, 0, 0, 0, 1, 0}),
        vga6::make_ga_point({0, 0, 0, 0, 0, 1})
    };

    // 删除已有文件
    if (filesystem::exists(file_save_path)) {
        filesystem::remove_all(file_save_path);
    }
    filesystem::create_directory(file_save_path);

    // 渲染循环
    for (uint render_cnt = 0; render_cnt < render_times; ++render_cnt) {
        uint render_index = render_cnt;
        if (render_index == 0) { render_index = render_times; }

        #define R distribution(engine)
        float render_t = static_cast<float>(render_index) / (render_times - 1);
        auto current_rotor = vga6::rotor_lerp(rotor_start, rotor_end, render_t);
        float6x6 transform_mat{
            // 将rotor作用于六个基向量
            #define MAKE_BASE_VEC(basis_idx) \
                vga6::to_float6( \
                    current_rotor * ga_basis[basis_idx] \
                        * current_rotor.reverse() * mat_coeff \
                )
            .col1=MAKE_BASE_VEC(0),
            .col2=MAKE_BASE_VEC(1),
            .col3=MAKE_BASE_VEC(2),
            .col4=MAKE_BASE_VEC(3),
            .col5=MAKE_BASE_VEC(4),
            .col6=MAKE_BASE_VEC(5)
            #undef MAKE_BASE_VEC
        };
        auto mb_z = complex(2, R);
        #undef R

        std::vector<std::byte> pixels{image_width * image_height * 4};
        Image<float> image = device.create_image<float>(PixelStorage::BYTE4, image_width, image_height);
        stream
            << main_shader(
                image,
                transform_mat,
                translate_vec,
                512
            ).dispatch(image_width, image_height)
            << image.copy_to(pixels.data())
            << synchronize();

        std::cout << (
            file_save_path / (std::to_string(render_index) + std::string(".png"))
        ).string().data() << "\n";

        char inp;
        if (render_index == 4) {
            std::cout << "want to continue(y/N)? ";
            std::cin >> inp;
            if (inp != 'y' && inp != 'Y') { exit(0); }
        }

        stbi_write_png(
            (
                file_save_path / (std::to_string(render_index) + std::string(".png"))
            ).string().data(),
            image_width, image_height, 4,
            pixels.data(), 0
        );
    }

    // 输出合成视频的命令
    std::cout
        << "ffmpeg -f image2 -i"
        << " \"" << (file_save_path / "%d.png").string() << "\""
        << " -vcodec libx264"
        << " -pix_fmt yuv420p -movflags +faststart -framerate 60"
        << " \"" << (filesystem::current_path() / "_111.mp4").string() << "\"";
}
