#pragma once

#include <mutex>
#include <vector>

namespace sph_harm {
    extern size_t lmax;
    extern std::vector<std::vector<double>> scales;
    extern std::mutex scales_mutex;
}
