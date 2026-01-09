#pragma once
#include <filesystem>
#include <string>
#include <typeindex>
#include <variant>
#include <vector>

namespace parser {
using ParamValueType = std::variant<
    int,
    float,
    bool,
    std::string,
    std::vector<int>,
    std::vector<float>,
    std::vector<std::string>
>;

struct Param {
    std::type_index type;
    std::string name;
    ParamValueType value;
};

struct SourceLocation {
    std::filesystem::path file_path;
    size_t line;
    size_t start_char;
};

class ASTNode {
    enum class Tag {
        // TODO(unybhgfd):
    };

    SourceLocation source_location_;

public:
    virtual void accept(class ASTVisitor& visitor) const = 0;
    [[nodiscard]] virtual std::string to_string() const = 0;
};

class Statement : public ASTNode {
    [[nodiscard]] std::string to_string() const override {
        return "Statement";
    }
};
} // namespace parser
