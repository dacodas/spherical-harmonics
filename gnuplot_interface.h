#pragma once

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <vector>

#include <gsl_math.h>
#include <gsl_sf_legendre.h>
#include <array>

#include "globals.h"


void init_domain_vector(std::vector<double> &vector, double start, double finish, size_t sample_points);

int gnuplot_task();
