#pragma once
#include <luisa/luisa-compute.h>
#include <memory>
#include <vector>
#include "basic.hpp"

namespace parser {
class BlockNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> children;

    void addChild(const std::shared_ptr<ASTNode>& child) {
        children.push_back(child);
    }
};

class LookAtStmt final : public ASTNode {
public:
    luisa::compute::float3 eye;  // 相机位置
    luisa::compute::float3 look; // 看向的点
    luisa::compute::float3 up;   // 向上位置

    [[nodiscard]] std::string to_string() const override {
        return luisa::format(FMT_STRING("LookAtStmt(eye = {}, look = {}, up = {})"), eye, look, up);
    }
};
} // namespace parser
