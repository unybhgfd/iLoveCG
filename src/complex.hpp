#pragma once
#include <luisa/luisa-compute.h>

using namespace luisa;
using namespace luisa::compute;

struct complex {
    float real, imag;
};

using Complex = Var<complex>;

[[nodiscard]] inline Complex operator*(Complex lhs, Complex rhs);

LUISA_STRUCT(complex, real, imag) {
    [[nodiscard]] Complex conjugate() const noexcept {
        return def<complex>(real, -imag);
    }

    [[nodiscard]] Float abs_square() const noexcept {
        return real * real + imag * imag;
    }

    [[nodiscard]] Complex log_natural() const noexcept {
        return def<complex>(
            log(sqrt(abs_square())),
            atan2(imag, real)
        );
    }

    [[nodiscard]] Complex exp() const noexcept {
        Float real_exp = luisa::compute::exp(real);
        return def<complex>(
            real_exp * cos(imag),
            real_exp * sin(imag)
        );
    }

    [[nodiscard]] Complex pow(const Complex& exponent) const noexcept {
        return (
            exponent * this->log_natural()
        )->exp();
    }
};

[[nodiscard]] inline Complex operator-(Complex val) {
    return def<complex>(-val.real, -val.imag);
}
/*!	This operator implements complex addition.*/
[[nodiscard]] inline Complex operator+(Complex lhs, Complex rhs) {
    return def<complex>(lhs.real + rhs.real, lhs.imag + rhs.imag);
}
/*!	This operator implements complex subtraction.*/
[[nodiscard]] inline Complex operator-(Complex lhs, Complex rhs) {
    return def<complex>(lhs.real - rhs.real, lhs.imag - rhs.imag);
}
/*!	This operator implements complex multiplication.*/
[[nodiscard]] inline Complex operator*(Complex lhs, Complex rhs) {
    return def<complex>(
        lhs.real * rhs.real - lhs.imag * rhs.imag,
        lhs.real * rhs.imag + lhs.imag * rhs.real
    );
}
/*!	This operator implements mixed real complex addition.*/
[[nodiscard]] inline Complex operator+(Float lhs, Complex rhs) {
    return def<complex>(lhs + rhs.real, rhs.imag);
}
/*!	This operator implements  mixed real complex multiplication.*/
[[nodiscard]] inline Complex operator*(Float lhs, Complex rhs) {
    return def<complex>(lhs * rhs.real, lhs * rhs.imag);
}
/*!	This operator implements mixed real complex addition.*/
[[nodiscard]] inline Complex operator+(float lhs, Complex rhs) {
    return def<complex>(lhs + rhs.real, rhs.imag);
}
/*!	This operator implements  mixed real complex multiplication.*/
[[nodiscard]] inline Complex operator*(float lhs, Complex rhs) {
    return def<complex>(lhs * rhs.real, lhs * rhs.imag);
}
[[nodiscard]] inline Complex operator/(Complex numerator, Complex denominator) {
    auto factor = 1.0f / denominator->abs_square();
    return def<complex>(
        (numerator.real * denominator.real + numerator.imag * denominator.imag) * factor,
        (-numerator.real * denominator.imag + numerator.imag * denominator.real) * factor
    );
}