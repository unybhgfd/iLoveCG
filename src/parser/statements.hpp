#pragma once
#include <luisa/luisa-compute.h>
#include "ast.hpp"

namespace parser {
class LookAtStmt final : public Statement {
public:
    luisa::compute::float3 eye;  // 相机位置
    luisa::compute::float3 look; // 看向的点
    luisa::compute::float3 up;   // 向上位置

    void accept(ASTVisitor& visitor) const override;
    [[nodiscard]] std::string to_string() const override {
        return luisa::format(FMT_STRING("LookAtStmt(eye = {}, look = {}, up = {})"), eye, look, up);
    }
};
} // namespace parser
