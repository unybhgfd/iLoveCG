#pragma once

#include <luisa/luisa-compute.h>

using namespace luisa;
using namespace luisa::compute;

namespace color {
constexpr float3x3 xyz_to_rec709 = transpose(float3x3{
    { 3.2409699419045213f, -1.5373831775700935f, -0.4986107602930033f},
    {-0.9692436362808798f,  1.8759675015077206f,  0.0415550574071756f},
    { 0.0556300796969936f, -0.2039769588889765f,  1.0569715142428784f}});
constexpr float3x3 rec709_to_xyz = inverse(xyz_to_rec709);

constexpr float3x3 xyz_to_rec2020 = transpose(float3x3{
    { 1.7166511880, -0.3556707838, -0.2533662814},
    {-0.6666843518,  1.6164812366,  0.0157685458},
    { 0.0176398574, -0.0427706133,  0.9421031212}});
constexpr float3x3 rec2020_to_xyz = inverse(xyz_to_rec2020);

// ACES色彩管理系统
// 参考: https://pub.smpte.org/doc/st2065-1/20200909-pub/
namespace aces {
constexpr float2 aces_wp = {0.32168, 0.33767}; // 白点

constexpr float3x3 xyz_to_ap0 = transpose(float3x3{
    {0.9525523959, 0.,            0.0000936786},
    {0.3439664498, 0.7281660966, -0.0721325464},
    {0.,           0.,            1.0088251844}});
constexpr float3x3 ap0_to_xyz = inverse(xyz_to_ap0);

constexpr float3x3 xyz_to_ap1 = transpose(float3x3{
    { 0.6624541811, 0.1340042065, 0.1561876870},
    { 0.2722287168, 0.6740817658, 0.0536895174},
    {-0.0055746495, 0.0040607335, 1.0103391003}});
constexpr float3x3 ap1_to_xyz = inverse(xyz_to_ap1);
}  // namespace aces

// 转换到xy色品图
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
}  // namespace color
