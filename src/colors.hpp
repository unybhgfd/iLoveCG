#pragma once

#include <luisa/luisa-compute.h>
#include <luisa/backends/ext/dx_hdr_ext.hpp>

using namespace luisa;
using namespace luisa::compute;

// 辅助函数
namespace color {
// 转换到xy色品图, 保留y分量
extern constexpr float3 XYZ_to_xyY(float3 xyz) noexcept {
    float divisor = xyz.x + xyz.y + xyz.z;
    if (divisor == 0.) { divisor = 1e-10; }
    return make_float3(
        xyz.x / divisor,
        xyz.y / divisor,
        xyz.y
    );
}

constexpr float3 xyY_to_XYZ(float3 xyy) noexcept {
    return make_float3(
        xyy.x * xyy.z / max(xyy.y, 1e-10F),
        xyy.y,
        (1.F - xyy.x - xyy.y) * xyy.z / max(xyy.y, 1e-10F)
    );
}

/**
 根据三原色和白点在xy色品图的坐标,
 计算 **从该颜色空间到CIE-XYZ** 的矩阵
 推导过程: https://chat.deepseek.com/share/3si694s6qqgfnqksx9
 */
constexpr float3x3 primes_white_point_to_matrix_to_xyz(
    const float2& red_xy,   // 三原色中红色在xy色品图中的位置
    const float2& green_xy, // 三原色中绿色在xy色品图中的位置
    const float2& blue_xy,  // 三原色中蓝色在xy色品图中的位置
    const float2& white_xy  // 白点在xy色品图中的位置
) {
    // 1. 计算白点的XYZ值 (假设Y_w = 1)
    float x_w = white_xy.x / white_xy.y;
    float y_w = 1.0f;
    float z_w = (1.0f - white_xy.x - white_xy.y) / white_xy.y;
    float3 white_xyz = {x_w, y_w, z_w};

    float3 v_r = {
        red_xy.x / red_xy.y,
        1.0f,
        (1.0f - red_xy.x - red_xy.y) / red_xy.y
    };

    // 绿原色
    float3 v_g = {
        green_xy.x / green_xy.y,
        1.0f,
        (1.0f - green_xy.x - green_xy.y) / green_xy.y
    };

    // 蓝原色
    float3 v_b = {
        blue_xy.x / blue_xy.y,
        1.0f,
        (1.0f - blue_xy.x - blue_xy.y) / blue_xy.y
    };

    // 3. 构造未缩放的基向量矩阵A (列存储)
    const float3x3 mat_a = {v_r, v_g, v_b};

    // 4. 求解缩放系数 S = [S_r, S_g, S_b]^T
    // 解方程: A * S = white_xyz
    // 由于矩阵A是3x3，我们可以求逆来解这个方程
    const float3x3 a_inv = inverse(mat_a);
    const float3 mat_s = a_inv * white_xyz;

    // 5. 构造RGB到XYZ的转换矩阵M (列存储)
    return float3x3{
        {v_r.x * mat_s.x, v_r.y * mat_s.x, v_r.z * mat_s.x},
        {v_g.x * mat_s.y, v_g.y * mat_s.y, v_g.z * mat_s.y},
        {v_b.x * mat_s.z, v_b.y * mat_s.z, v_b.z * mat_s.z}
    };
}
}  // namespace color


// 颜色空间
namespace color {
constexpr float3x3 kXyzToRec709 = transpose(float3x3{
    { 3.2409699419045213f, -1.5373831775700935f, -0.4986107602930033f},
    {-0.9692436362808798f,  1.8759675015077206f,  0.0415550574071756f},
    { 0.0556300796969936f, -0.2039769588889765f,  1.0569715142428784f}});
constexpr float3x3 kRec709ToXyz = inverse(kXyzToRec709);
constexpr float3 kRec709WhitePoint = XYZ_to_xyY(kRec709ToXyz * float3(1));

constexpr float3x3 kXyzToRec2020 = transpose(float3x3{
    { 1.7166511880, -0.3556707838, -0.2533662814},
    {-0.6666843518,  1.6164812366,  0.0157685458},
    { 0.0176398574, -0.0427706133,  0.9421031212}});
constexpr float3x3 kRec2020ToXyz = inverse(kXyzToRec2020);
constexpr float3 kRec2020WhitePoint = XYZ_to_xyY(kRec2020ToXyz * float3(1));

// ACES色彩管理系统
// 参考: https://draftdocs.acescentral.com/home-technical/
// ap0和ap1改成ap zero和ap one防止混淆
namespace aces {
// aces白点在xy色品图上坐标. ap0和ap1都是用这个白点
constexpr float2 kAcesWhitePoint = {0.32168, 0.33767};

// primes指的是三原色

// ap0三原色在xy色品图的坐标
constexpr std::array<float2, 3> kApZeroPrimes = {{
    {0.73470, 0.26530},
    {0.00000, 1.000},
    {0.00010, -0.077}
}};

// ap1三原色在xy色品图的坐标
constexpr std::array<float2, 3> kApOnePrimes = {{
    {0.713, 0.293},
    {0.165, 0.830},
    {0.128, 0.044}
}};

constexpr float3x3 kApZeroToXyz = primes_white_point_to_matrix_to_xyz(
    kApZeroPrimes[0], kApZeroPrimes[1], kApZeroPrimes[2],
    kAcesWhitePoint
);
constexpr float3x3 kXyzToApZero = inverse(kApZeroToXyz);

constexpr float3x3 kApOneToXyz = primes_white_point_to_matrix_to_xyz(
    kApOnePrimes[0], kApOnePrimes[1], kApOnePrimes[2],
    kAcesWhitePoint
);
constexpr float3x3 kXyzToApOne = inverse(kApOneToXyz);
}  // namespace aces
}  // namespace color


// 色貌模型
namespace color::appearance {
/*
CAM16
https://www.researchgate.net/publication/318152296_Comprehensive_color_solutions_CAM16_CAT16_and_CAM16-UCS
https://www.researchgate.net/publication/220865484_Usage_guidelines_for_CIECAM97s
https://doi.org/10.1002/col.22131
https://observablehq.com/@jrus/cam16
https://arxiv.org/abs/1802.06067

CAM16 Corrections: Hellwig and Fairchild
http://markfairchild.org/PDFs/PAP45.pdf

Helmholtz Kohlrausch Effect extension: Hellwig, Stolitzka, and Fairchild
https://doi.org/10.1002/col.22793
*/
}  // namespace color::appearance
