#include <iostream>
#include <ostream>
#include <source_location>
#include <corecrt_math.h>
#include <luisa/luisa-compute.h>
#include <stb/stb_image_write.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include "common/tiny_obj_loader.h"
#include "common/happly.h"
// #include "complex.hpp"
#include "colors.hpp"

using namespace luisa;
using namespace luisa::compute;

int main() {
    std::cout << to_string(color::aces::kApZeroToXyz) << "\n";
}
