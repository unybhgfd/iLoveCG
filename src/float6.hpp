#pragma once
#include <luisa/luisa-compute.h>

using namespace luisa;
using namespace luisa::compute;

struct float6 {
    float3 first;
    float3 second;
};
LUISA_STRUCT(float6, first, second) {};
using Float6 = Var<float6>;

[[nodiscard]] float6 make_float6(
    float a, float b, float c,
    float d, float e, float f
) {
    float6 result;
    result.first = make_float3(a, b, c);
    result.second = make_float3(d, e, f);
    return result;
}

[[nodiscard]] float6 make_float6(
    float2 vec, float c,
    float d, float e, float f
) {
    float6 result;
    result.first = make_float3(vec, c);
    result.second = make_float3(d, e, f);
    return result;
}

struct float6x6 {
    float6 col1; float6 col2;
    float6 col3; float6 col4;
    float6 col5; float6 col6;
};
LUISA_STRUCT(float6x6,
    col1, col2,
    col3, col4,
    col5, col6
) {};
using Float6x6 = Var<float6x6>;


[[nodiscard]] inline Float6 operator+(Float6 lhs, Float6 rhs) {
    return def<float6>(
        lhs.first + rhs.first,
        lhs.second + rhs.second
    );
}

[[nodiscard]] inline Float6 operator*(Float lhs, Float6 rhs) {
    return def<float6>(
        lhs * rhs.first,
        lhs * rhs.second
    );
}

[[nodiscard]] inline Float6 operator*(Float6 lhs, Float rhs) { return rhs * lhs; }

[[nodiscard]] inline Float6 operator*(Float6x6 lhs, Float6 rhs) {
    return lhs.col1 * rhs.first.x +
           lhs.col2 * rhs.first.y +
           lhs.col3 * rhs.first.z +
           lhs.col4 * rhs.second.x +
           lhs.col5 * rhs.second.y +
           lhs.col6 * rhs.second.z;
}
