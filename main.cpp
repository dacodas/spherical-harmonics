#include <stdio.h>
#include <stdint.h>
#include <vector>

#include <gsl_math.h>
#include <gsl_sf_legendre.h>
#include <array>

void init_domain_vector(std::vector<double> &vector, double start, double finish, size_t sample_points)
{
    vector.reserve(sample_points);
    double stride = (finish - start) / sample_points;

    for (double member = start; member <= finish; member += stride)
    {
        vector.push_back(member);
    }
}

int main()
{
    std::vector<double> theta, phi;

    // Number of samples for chosen range of theta and phi
    size_t sample_points = 100;

    // Highest index of l we're going to calculate
    size_t lmax = 2;

    // Add a bit of an overshoot or else the surface won't be closed in gnuplot
    init_domain_vector(theta, 0, M_PI + .1, sample_points);
    init_domain_vector(phi, 0, 2 * M_PI + .1, sample_points);


    // Figure out how many Legendre polynomials are actually being calculated
    size_t theta_result_size = gsl_sf_legendre_array_n(lmax);
    std::vector<std::vector<double>> theta_result(sample_points, std::vector<double>(theta_result_size));
    std::vector<std::vector<double>> phi_result(sample_points, std::vector<double>(lmax+1));

    // Calculate the normalized Legendre polynomials and the complex exponential for each phi and theta
    double temp_result [theta_result_size];
    for ( size_t idx_th = 0; idx_th < sample_points; ++idx_th )
    {
        // printf("Calculating Legendre polynomials for point number %d (%.3f)\n", idx_th, theta[idx_th]);
        gsl_sf_legendre_array(GSL_SF_LEGENDRE_SPHARM, lmax, cos(theta[idx_th]), theta_result[idx_th].data());

        for ( size_t idx_ph = 0; idx_ph < sample_points; ++idx_ph )
        {
            for ( size_t m = 0; m <= lmax; ++m )
            {
                // printf("Calculating phi dependent sinusoid for m=%d %d (%.3f)\n", m, idx_ph, phi[idx_ph]);
                phi_result[idx_ph][m] = cos(m*phi[idx_ph]);
            }
        }
    }

    std::vector<std::vector<double>> scales(lmax+1, std::vector<double>(lmax+1, 0.0));
    scales[2][2] = 1;

    const double sphere_r = 5;
    // Calculate the actual value for the current time slice
    for ( size_t idx_th = 0; idx_th < sample_points; ++idx_th )
    {
        for (size_t idx_ph = 0; idx_ph < sample_points; ++idx_ph )
        {
            double cur_val = 0;
            for (size_t l = 0; l <= lmax; ++l)
                for (size_t m = 0; m <= lmax; ++m)
                {
                    size_t idx_th_res = gsl_sf_legendre_array_index(l, m);
                    cur_val += scales[l][m] * phi_result[idx_ph][m] * theta_result[idx_th][idx_th_res];
                }
            printf("%.3f %.3f %.3f %.3f\n", theta[idx_th], phi[idx_ph], sphere_r + cur_val, cur_val);
        }
        printf("\n");
    }

    // set mapping spherical
    // set term qt noraise
    // splot 'data' u 1:2:(radius+$3):3 with pm3d
    // set cbrange[-1:1]
}
