// 有一元 N 次方程一般式, 这个方程的每个系数有可能是 A , 不然就是 B .
// 我们遍历所有的可能, 计算方程的所有根.
// 把遍历出的根的集合的每个元素 a+bi 都作为一个点 (a, b) 放在画布上,
// N较大时形成分形图案.

#include <complex>
#include <luisa/luisa-compute.h>

// using namespace luisa;
// using namespace luisa::compute;
using CpuFloatingType = long double;
constexpr unsigned EquationN = 7; // 方程次数
constexpr std::complex<CpuFloatingType> EquationA = {0, 1}; // 调整这个 A 和 B 可以改变分形形状
constexpr std::complex<CpuFloatingType> EquationB = {0, -1};
constexpr struct {
    CpuFloatingType real_min = -2.;
    CpuFloatingType real_max = 2.;
    CpuFloatingType imag_min = -2.;
    CpuFloatingType imag_max = 2.;
} CanvasInfo;
constexpr struct {
    unsigned width = 1024;
    unsigned height = 1024;
} ImageInfo;

// Aberth–Ehrlich method 求多项式所有根
std::array<std::complex<CpuFloatingType>, EquationN> aberth(
    std::array<std::complex<CpuFloatingType>, EquationN+1> coefficients // 加上常数项就是 N+1 个系数
) {
    // TODO(unybhgfd):
}
