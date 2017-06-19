#include "gnuplot_interface.h"

void init_domain_vector(std::vector<double> &vector, double start, double finish, size_t sample_points)
{
    vector.reserve(sample_points);
    double stride = (finish - start) / sample_points;

    for (double member = start; member <= finish; member += stride)
    {
        vector.push_back(member);
    }
}

int gnuplot_task()
{
    std::vector<double> theta, phi;

    // Number of samples for chosen range of theta and phi
    size_t sample_points = 100;

    // Add a bit of an overshoot or else the surface won't be closed in gnuplot
    init_domain_vector(theta, 0, M_PI + .1, sample_points);
    init_domain_vector(phi, 0, 2 * M_PI + .1, sample_points);

    // Figure out how many Legendre polynomials are actually being calculated
    size_t theta_result_size = gsl_sf_legendre_array_n(sph_harm::lmax);
    std::vector<std::vector<double>> theta_result(sample_points, std::vector<double>(theta_result_size));
    std::vector<std::vector<double>> phi_result(sample_points, std::vector<double>(sph_harm::lmax+1));

    // Calculate the normalized Legendre polynomials and the complex exponential for each phi and theta
    double temp_result [theta_result_size];
    for ( size_t idx_th = 0; idx_th < sample_points; ++idx_th )
    {
        // printf("Calculating Legendre polynomials for point number %d (%.3f)\n", idx_th, theta[idx_th]);
        gsl_sf_legendre_array(GSL_SF_LEGENDRE_SPHARM, sph_harm::lmax, cos(theta[idx_th]), theta_result[idx_th].data());

        for ( size_t idx_ph = 0; idx_ph < sample_points; ++idx_ph )
        {
            for ( size_t m = 0; m <= sph_harm::lmax; ++m )
            {
                // printf("Calculating phi dependent sinusoid for m=%d %d (%.3f)\n", m, idx_ph, phi[idx_ph]);
                phi_result[idx_ph][m] = cos(m*phi[idx_ph]);
            }
        }
    }

    const double sphere_r = 3;
    const double range = 5;
    std::vector<std::array<double, 3>> non_zero_scales = {
	// l, m, scale_factor
	{3, 5, .5},
	{5, 5, 1}
    };

    for ( auto scale : non_zero_scales )
    {
	sph_harm::scales[scale[0]][scale[1]] = scale[2];
    }
    
    FILE *pipe = popen("gnuplot -persist 2>&1 > /dev/null", "w");
    // TODO: Change the pgid of the above process so that it doesn't
    // get SIGINT when Ctrl-C is pressed

    fprintf(pipe, "set mapping spherical\n");
    fprintf(pipe, "set term qt noraise\n");
    fprintf(pipe, "set pm3d depthorder\n");
    fprintf(pipe, "set cbrange [-1:1]\n");
    fprintf(pipe, "set xrange [%.3f:%.3f]\n", -range, range);
    fprintf(pipe, "set yrange [%.3f:%.3f]\n", -range, range);
    fprintf(pipe, "set zrange [%.3f:%.3f]\n", -range, range);

    double t = 0;
    while (true) {
        fprintf(pipe, "$data << EOD\n");

	std::unique_lock<std::mutex> guard(sph_harm::scales_mutex);
	auto local_scales = sph_harm::scales;
	guard.unlock();

        // Calculate the actual value for the current time slice
        for (size_t idx_th = 0; idx_th < sample_points; ++idx_th) {
            for (size_t idx_ph = 0; idx_ph < sample_points; ++idx_ph) {
                double cur_val = 0;
                for (size_t l = 0; l <= sph_harm::lmax; ++l)
                    for (size_t m = 0; m <= sph_harm::lmax; ++m) {
                        size_t idx_th_res = gsl_sf_legendre_array_index(l, m);
                        cur_val += cos(4*t) * local_scales[l][m] * phi_result[idx_ph][m] * theta_result[idx_th][idx_th_res];
                    }
                fprintf(pipe, "%.3f %.3f %.3f %.3f\n", theta[idx_th], phi[idx_ph], sphere_r + cur_val, cur_val);
            }
            fprintf(pipe, "\n");
	} 

        fprintf(pipe, "EOD\n");
        fprintf(pipe, "splot $data w pm3d\n");

        t += .1;
    } 

    pclose(pipe);
}
