#include <stdio.h>
#include <stdint.h>
#include <vector>

#include <gsl_math.h>
#include <gsl_sf_legendre.h>

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
    size_t sample_points = 100;

    std::vector<double> theta, phi, theta_result, phi_result;

    // Add a bit of an overshoot or else the surface won't be closed in gnuplot
    init_domain_vector(theta, 0, M_PI + 1, sample_points);
    init_domain_vector(phi, 0, 2 * M_PI + 1, sample_points);
    theta_result.reserve(sample_points);
    phi_result.reserve(sample_points);


    uint32_t m = 2;
    // TODO: Start using the gsl_sf_legendre_array function
    // Get values for theta
    for ( double x : theta )
    {
        double result = gsl_sf_legendre_sphPlm(2, m, cos(x));
        theta_result.push_back(result);
    }

    for ( double x : phi )
    {
        phi_result.push_back(cos(m*x));
    }

    size_t theta_counter = 0;
    size_t phi_counter = 0;

    for ( size_t th = 0; th < sample_points; ++th )
    {
        for ( size_t ph = 0; ph < sample_points; ++ph )
        {
            printf("%.3f %.3f %.3f\n", theta[th], phi[ph], theta_result[th]*phi_result[ph]);
        }

        // gnuplot's pm3d needs newlines whenever the first variable changes
    	printf("\n");
    }

    // set mapping spherical
    // set term qt noraise
    // splot 'data' u 1:2:(radius+$3):3 with pm3d
    // set cbtics
}
