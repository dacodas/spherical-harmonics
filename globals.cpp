#include "globals.h"

size_t sph_harm::lmax = 5;

std::vector<std::vector<double>> sph_harm::scales(sph_harm::lmax+1, std::vector<double>(lmax+1, 0.0));

std::mutex sph_harm::scales_mutex;

