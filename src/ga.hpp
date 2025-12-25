#pragma once

#include <array>
#include <bit>
#include <iostream>
#include <numbers>
#include <random>
#include <string>
#include <type_traits>

#include <luisa/luisa-compute.h>
#include "float6.hpp"

using namespace luisa;
using namespace luisa::compute;

namespace {
constexpr size_t log2(size_t n) {
    if (n == 0) { return 0; }
    size_t log = 0;
    while (n >>= 1) { ++log; }
    return log;
}

template <typename T, size_t NBase, std::array<T, NBase> BaseSquares>
concept ga_concept = requires() {
    requires NBase <= sizeof(size_t) * 8;
    requires std::is_floating_point_v<T>;
};
}  // namespace

template <
    typename T,                      // 系数的类型
    size_t NBase,                    // 基个数
    std::array<T, NBase> BaseSquares // 基的平方，正常情况下是-1, 0或1
>
    requires ga_concept<T, NBase, BaseSquares>
class GeoAlg {
private:
    static constexpr std::pair<T, size_t> get_mul_table(size_t idx_a, size_t idx_b) {
        size_t result_base = idx_a ^ idx_b;
        T sign = 1;
        std::array<size_t, NBase*2> mul{};
        size_t mul_size = 0;

        // 把idx_a, idx_b中的单个基放到mul里, 如e_{014} ==> {0, 1, 4}
        for (size_t i = 0; i < NBase; ++i) {
            if (idx_a & (1 << i)) {
                mul[mul_size++] = i;
            }
        }
        for (size_t i = 0; i < NBase; ++i) {
            if (idx_b & (1 << i)) {
                mul[mul_size++] = i;
            }
        }

        // 用冒泡排序计算sign
        if (mul_size > 1) {
            for (size_t i = 0; i < mul_size-1; ++i) {
                for (size_t j = 0; j < mul_size-i-1; ++j) {
                    if (mul[j] > mul[j+1]) {
                        auto tmp = mul[j];
                        mul[j] = mul[j+1];
                        mul[j+1] = tmp;
                        sign *= -1;
                    }
                }
            }
            for (size_t i = 0; i < mul_size-1; ++i) {
                if (mul[i] == mul[i+1]) {
                    sign *= BaseSquares[mul[i]];
                }
            }
        }

        return {sign, result_base};
    }

public:
    static constexpr size_t BasesCnt = 1 << NBase /* 实数1这里也视为一个基 */;
    using this_type = GeoAlg<T, NBase, BaseSquares>;

    /* data数组的第i项代表基i的系数
     * 若i除以(2的n次方)的余数为1则表示数组索引为n的基在多项式this的当前项中
     * i为0代表标量部分
     */
    std::array<T, BasesCnt> data{0.};

    /* MulTable结构：mult_table[a][b] = {sign, base}
     * 其中sign是积的符号(-1, 0, 1中的一个), base是结果的基(data数组的索引)
     */
    inline const static
    std::array<std::array<std::pair<T, size_t>, BasesCnt>, BasesCnt>
    MulTable = [] {
        std::array<std::array<std::pair<T, size_t>, BasesCnt>, BasesCnt> table{};

        for (size_t idx_a = 0; idx_a < BasesCnt; ++idx_a) {
            for (size_t idx_b = 0; idx_b < BasesCnt; ++idx_b) {
                table[idx_a][idx_b] = get_mul_table(idx_a, idx_b);
            }
        }

        return table;
    }();

    // 伪标量的平方
    constexpr static T PseudoscalarSquare = get_mul_table(BasesCnt-1, BasesCnt-1).first;

private:
    static std::string get_basis_name(size_t basis_index) {
        if (basis_index == 0) {
            return ""; // 标量部分
        }
        if (basis_index == BasesCnt-1) {
            return "I"; // 伪标量
        }

        std::string result = "e";
        result += "_{";
        bool first = true;
        for (size_t i = 0; i < NBase; ++i) {
            if (basis_index & (1 << i)) {
                if (!first) {
                    result += ",";
                }
                result += std::to_string(i);
                first = false;
            }
        }
        result += "}";
        return result;
    }

    static this_type pseudo_scalar_inverse() {
        static const this_type Inverse = []() {
            this_type result;
            result.data[BasesCnt-1] = 1;
            return result;
        }().inverse();

        return Inverse;
    }

public:
    GeoAlg()=default;

    // 从标量构造
    explicit GeoAlg(T scalar) {
        data[0] = scalar;
    };

    // 加法
    this_type operator+(const this_type& other) const {
        this_type result;
        for (size_t i = 0; i < BasesCnt; ++i) {
            result.data[i] = data[i] + other.data[i];
        }
        return result;
    }

    // 取负
    this_type operator-() const {
        this_type result;
        for (size_t i = 0; i < BasesCnt; ++i) {
            result.data[i] = -data[i];
        }
        return result;
    }

    // 减法
    this_type operator-(const this_type& other) const {
        return *this + (-other);
    }

    // 几何积
    [[nodiscard]] this_type operator*(const this_type& other) const {
        this_type result;
        for (size_t idx_a = 0; idx_a < BasesCnt; ++idx_a) {
            if (data[idx_a] == 0) { continue;}  // 0乘任何数都是0, 跳过
            for (size_t idx_b = 0; idx_b < BasesCnt; ++idx_b) {
                if (other.data[idx_b] == 0) { continue; }  // 同上
                const std::pair<T, size_t>& product = MulTable[idx_a][idx_b];
                T sign = product.first;

                result.data[product.second] += sign * data[idx_a] * other.data[idx_b];
            }
        }
        return result;
    }

    // 乘以系数
    [[nodiscard]] this_type operator*(T scalar) const {
        this_type result;
        for (size_t i = 0; i < BasesCnt; ++i) {
            result.data[i] = data[i] * scalar;
        }
        return result;
    }

    // 乘以系数
    [[nodiscard]] friend this_type operator*(const T& scalar, const this_type& val) {
        return val * scalar;
    }

    // 输出
    friend std::ostream& operator<<(std::ostream& stream, const this_type& obj) {
        bool first_term = true;
        bool all_zero = true;

        for (size_t i = 0; i < BasesCnt; ++i) {
            if (std::abs(obj.data[i]) > 1e-10) { // 非0
                all_zero = false;

                if (!first_term) {
                    if (obj.data[i] > 0) {
                        stream << " + ";
                    } else {
                        stream << " - ";
                    }
                } else {
                    if (obj.data[i] < 0) {
                        stream << "-";
                    }
                }

                T abs_value = std::abs(obj.data[i]);
                std::string basis_name = get_basis_name(i);

                if (basis_name.empty()) {
                    // 标量部分
                    stream << abs_value;
                } else {
                    if (abs_value != T(1)) {
                        stream << abs_value;
                    }
                    stream << basis_name;
                }

                first_term = false;
            }
        }

        if (all_zero) {
            stream << "0";
        }
        return stream;
    }

    // grade projection运算符. <A>_k 是提取A的k-向量部分
    [[nodiscard]] this_type grade_projection(size_t grade) const {
        this_type result;
        for (size_t i = 0; i < BasesCnt; ++i) {
            if (std::popcount(i) == grade) {
                result.data[i] = data[i];
            } else {
                result.data[i] = T(0);
            }
        }
        return result;
    }

    // 反转：反转所有基向量的顺序
    // 对于k-向量的reverse为(-1)^{k(k-1)/2}
    [[nodiscard]] this_type reverse() const {
        this_type result;

        for (size_t i = 0; i < BasesCnt; ++i) {
            if (data[i] == 0) { continue; }

            // 计算阶数grade（二进制表示中1的个数）
            size_t grade = std::popcount(i);

            // 反转运算的符号：(-1)^{grade(grade-1)/2}
            T sign = (grade % 4 == 0 || grade % 4 == 1) ? T(1) : T(-1);

            // 应用符号
            result.data[i] = sign * data[i];
        }

        return result;
    }

    // 模长的平方
    [[nodiscard]] T norm_squared() const {
        this_type rev = this->reverse();
        this_type product = *this * rev;
        return product.data[0];  // 标量部分
    }

    // 模长
    [[nodiscard]] T norm() const {
        return std::sqrt(std::abs(norm_squared()));
    }

    // 共轭
    [[nodiscard]] this_type conjugate() const {
        return this->reverse().grade_involution();
    }

    // 分级对合: 每个k-向量乘以(-1)^k
    [[nodiscard]] this_type grade_involution() const {
        this_type result;

        for (size_t i = 0; i < BasesCnt; ++i) {
            if (data[i] == 0) { continue; }

            // 计算阶数k
            size_t grade = std::popcount(i);

            // 分级对合的符号：(-1)^k
            T sign = (grade % 2 == 0) ? T(1) : T(-1);

            result.data[i] = sign * data[i];
        }

        return result;
    }

    // 逆: A^{-1} = reverse(A) / (A * reverse(A)). 这里不处理除0错误
    [[nodiscard]] this_type inverse() const {
        T norm_squared = this->norm_squared();

        this_type rev = this->reverse();
        this_type result;

        T inv_n2 = T(1) / norm_squared;
        for (size_t i = 0; i < BasesCnt; ++i) {
            result.data[i] = rev.data[i] * inv_n2;
        }

        return result;
    }

    // e^A
    [[nodiscard]] this_type exp(size_t max_terms = 20) const {
        this_type result{1.};

        this_type term = *this;
        this_type accum = result + term;

        for (size_t iteration = 2; iteration <= max_terms; ++iteration) {
            term = term * (*this) * (T(1.) / T(iteration)); // A^n / n!
            accum = accum + term;

            // 如果收敛
            T term_norm = term.norm_squared();
            if (term_norm < T(1e-10)) {
                break;
            }
        }

        return accum;
    }

    // 内积. <A>_a dot <B>_b = <AB>_{|a - b|}
    [[nodiscard]] this_type dot(const this_type& other) const {
        this_type result{};

        for (size_t idx_a = 0; idx_a < BasesCnt; ++idx_a) {
            T coeff_a = data[idx_a];
            if (coeff_a == T(0)) { continue; }
            auto grade_a = std::popcount(idx_a);

            for (size_t idx_b = 0; idx_b < BasesCnt; ++idx_b) {
                T coeff_b = other.data[idx_b];
                if (coeff_b == T(0)) { continue; }
                auto grade_b = std::popcount(idx_b);

                const std::pair<T, size_t>& product = MulTable[idx_a][idx_b];
                size_t result_idx = product.second;
                size_t result_grade = std::popcount(result_idx);

                // 保留 |grade_a - grade_b| 阶部分
                if (result_grade == std::abs(grade_a - grade_b)) {
                    result.data[result_idx] += product.first * coeff_a * coeff_b;
                }
            }
        }
        return result;
    }

    // 外积. <A>_a wedge <B>_b = <AB>_{a + b}
    [[nodiscard]] this_type wedge(const this_type& other) const {
        this_type result{};

        for (size_t idx_a = 0; idx_a < BasesCnt; ++idx_a) {
            T coeff_a = data[idx_a];
            if (coeff_a == T(0)) { continue; }
            auto grade_a = std::popcount(idx_a);

            for (size_t idx_b = 0; idx_b < BasesCnt; ++idx_b) {
                T coeff_b = other.data[idx_b];
                if (coeff_b == T(0)) { continue; }
                auto grade_b = std::popcount(idx_b);

                const std::pair<T, size_t>& product = MulTable[idx_a][idx_b];
                size_t result_idx = product.second;
                size_t result_grade = std::popcount(result_idx);

                // 保留 grade_a + grade_b 阶部分
                if (result_grade == grade_a + grade_b) {
                    result.data[result_idx] += product.first * coeff_a * coeff_b;
                }
            }
        }
        return result;
    }

    // 对偶. dual A = A dot I^{-1}
    [[nodiscard]] this_type dual() const {
        static_assert(PseudoscalarSquare != 0., "伪标量平方为0时dual函数无定义");
        return this->dot(pseudo_scalar_inverse());
    }

    // regressive积. A vee B = dual(dual(A) wedge dual(b))
    [[nodiscard]] this_type regressive(const this_type& other) const {
        return (
            this->dual().wedge(
                other.dual()
            )
        ).dual();
    }
};

namespace vga6 {
    // 六维VGA
    using ga_type = GeoAlg<double, 6, {-1, -1, -1, -1, -1, -1}>;

    ga_type _vga_log(const ga_type& val) {
        // 提取标量部分（偶数阶部分）
        const double scalar = val.data[0];

        // 提取二重向量部分（2-向量）
        ga_type bivector = val.grade_projection(2);

        // 计算二重向量的模长平方
        const double b2_norm = bivector.norm_squared();

        if (b2_norm > 0) {
            // 正常旋转（欧几里得空间）
            const double theta = 2.0 * std::acos(std::clamp(scalar, -1., 1.));
            const double scale = theta / std::sin(theta / 2.0);
            return bivector * (0.5 * scale);
        }
        // 双曲旋转（闵可夫斯基空间），这里简化为正常处理
        const double arg = std::clamp(scalar, -1., 1.);
        const double theta = 2.0 * std::acosh(arg);
        const double scale = theta / std::sinh(theta / 2.0);
        return bivector * (0.5 * scale);
    }

    ga_type random_rotor() {
        static std::random_device random_device;
        static std::mt19937 gen(random_device());
        static std::uniform_real_distribution<double> dist(-1.0, 1.0);
        static std::uniform_real_distribution<double> angle_dist(0.0, 2.0 * std::numbers::pi);

        // 生成随机角度
        double angle = angle_dist(gen);

        // 生成随机的单位二重向量
        ga_type bivector{};

        // 在6维空间中，有C(6,2)=15个基二重向量
        // 我们生成15个随机系数，然后归一化
        double norm_sq = 0.0;
        for (size_t i = 0; i < ga_type::BasesCnt; ++i) {
            if (std::popcount(i) == 2) {  // 只处理二重向量基
                double coeff = dist(gen);
                bivector.data[i] = coeff;
                norm_sq += coeff * coeff;
            }
        }

        // 归一化二重向量
        if (norm_sq > 1e-10) {
            double inv_norm = 1.0 / std::sqrt(norm_sq);
            for (size_t i = 0; i < ga_type::BasesCnt; ++i) {
                if (std::popcount(i) == 2) {
                    bivector.data[i] *= inv_norm;
                }
            }
        } else {
            // 如果随机数太小，使用默认二重向量
            bivector.data[0b000011] = 1.0;  // e_{0,1}
        }

        // 计算旋量：exp(angle/2 * bivector)
        ga_type rotor = (bivector * (angle * 0.5)).exp();

        // 归一化旋量以确保数值稳定性
        return rotor * (1. / rotor.norm());
    }

    ga_type rotor_lerp(const ga_type& rotor, double times) {
        // 实现旋量的球面线性插值（slerp）
        // slerp(R0, R1, t) = R0 * exp(t * log(R0^{-1} * R1))

        // 单位旋量
        static const ga_type IdentityRotor{1.0};

        // 计算相对旋转：R_rel = rotor0^{-1} * rotor
        // 这里假设从单位旋量插值到目标旋量
        ga_type r_rel = rotor;  // identity_rotor.inverse() * rotor = rotor

        // 计算相对旋转的对数
        ga_type log_r_rel = _vga_log(r_rel);

        // 插值：exp(t * log_R_rel)
        ga_type interpolated = (log_r_rel * times).exp();

        // 归一化结果
        return interpolated * (1. / interpolated.norm());
    }

    ga_type rotor_lerp(const ga_type& start, const ga_type& end, double t_val) {
        t_val = std::clamp(t_val, 0.0, 1.0);

        // 计算相对旋量：end * start^{-1}
        ga_type start_inv = start.inverse();
        ga_type relative_rotor = end * start_inv;

        // 计算相对旋量的对数（得到一个二重向量）
        ga_type log_relative = _vga_log(relative_rotor);

        // 插值：exp(t * log(relative))
        ga_type interpolated_log = log_relative * t_val;
        ga_type interpolated_rotor = interpolated_log.exp();

        // 应用插值：start * interpolated_rotor
        ga_type result = start * interpolated_rotor;

        // 归一化结果以确保数值稳定性
        double norm = result.norm();
        result = result * (1.0 / norm);

        return result;
    }

    [[nodiscard]] inline float6 to_float6(ga_type val) {
        return make_float6(
            val.data[0b000001],
            val.data[0b000010],
            val.data[0b000100],
            val.data[0b001000],
            val.data[0b010000],
            val.data[0b100000]
        );
    }

    inline ga_type make_ga_point(std::array<double, 6> pos) {
        ga_type result;
        result.data[0b000001] = pos[0];
        result.data[0b000010] = pos[1];
        result.data[0b000100] = pos[2];
        result.data[0b001000] = pos[3];
        result.data[0b010000] = pos[4];
        result.data[0b100000] = pos[5];
        return result;
    }
}  // namespace vga6
