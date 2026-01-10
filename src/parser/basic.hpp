#pragma once
#include <filesystem>
#include <string>
#include <typeindex>
#include <utility>
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
    enum class Tag : std::uint8_t {
        kInteger,
        kFloat,
        kPoint2,
        kPoint3,
        kVector3,
        kNormal3,
        kSpectrum,
        kRgb,
        kBlackbody,
        kBool,
        kString
    };

    std::type_index type;
    std::string name;
    ParamValueType value;
};

struct SourceLocation {
    std::filesystem::path file_path;
    size_t line;
    size_t start_char;
};

struct ASTVisitor;

class ASTNode {
public:
    enum class Tag : std::uint8_t {
        kReflectance,
        kEmission,
        kTranslate,
        kScale,
        kRotate,
        kLookAt,
        kCoordinateSystem,
        kCoordSysTransform,
        kTransform,
        kConcatTransform,
        kInclude,
    };

    SourceLocation source_location_;
    Tag tag_;

    [[nodiscard]] virtual std::string to_string() const = 0;

    ASTNode(Tag tag, SourceLocation location) : tag_(tag), source_location_(std::move(location)) {}
};
} // namespace parser
