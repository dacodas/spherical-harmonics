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
    
    // FILE *pipe = popen("gnuplot -persist 2>&1 > /dev/null", "w");
    // TODO: Look into blocking SIGINT as I'm forking and whatnot
    int ptc_pipe[2];
    int ctp_stdout_pipe[2];
    int ctp_stderr_pipe[2];
    if ( pipe(ptc_pipe) != 0 ||
	 pipe(ctp_stdout_pipe) != 0 ||
	 pipe(ctp_stderr_pipe) != 0 )
    {
	printf("FATAL error: couldn't create pipes to gnuplot!\n");
	exit(1);
    }

    pid_t gnuplot_pid = fork();

    // If this is the child process...
    if ( gnuplot_pid == 0 )
    {
	// Close write-end of ptc_pipe and read ends of stdout and
	// stderr pipes
	close(ptc_pipe[1]);
	close(ctp_stdout_pipe[0]);
	close(ctp_stderr_pipe[0]);

	dup2(ptc_pipe[0], STDIN_FILENO);
	dup2(ctp_stdout_pipe[1], STDOUT_FILENO);
	dup2(ctp_stderr_pipe[1], STDERR_FILENO);

	if ( setpgid(0, 0) != 0 )
	{
	    printf("Error: unable to set pgid of gnuplot child process!\n");
	    exit(1);
	}
	execl("/usr/sbin/gnuplot", "gnuplot", "-persist", (char *) NULL);
    }

    // Close read-end of ptc_pipe and write ends of stdout and stderr
    // pipes
    close(ptc_pipe[0]);
    close(ctp_stdout_pipe[1]);
    close(ctp_stderr_pipe[1]);
    if ( setpgid(gnuplot_pid, gnuplot_pid) != 0 )
    {
	printf("Error: unable to set pgid of the gnuplot parent process!\n");
    }

    dprintf(ptc_pipe[1], "set mapping spherical\n");
    dprintf(ptc_pipe[1], "set term qt noraise\n");
    dprintf(ptc_pipe[1], "set pm3d depthorder\n");
    dprintf(ptc_pipe[1], "set cbrange [-1:1]\n");
    dprintf(ptc_pipe[1], "set xrange [%.3f:%.3f]\n", -range, range);
    dprintf(ptc_pipe[1], "set yrange [%.3f:%.3f]\n", -range, range);
    dprintf(ptc_pipe[1], "set zrange [%.3f:%.3f]\n", -range, range);

    double t = 0;
    while (true) {
        dprintf(ptc_pipe[1], "$data << EOD\n");

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
                dprintf(ptc_pipe[1], "%.3f %.3f %.3f %.3f\n", theta[idx_th], phi[idx_ph], sphere_r + cur_val, cur_val);
            }
            dprintf(ptc_pipe[1], "\n");
	} 

        dprintf(ptc_pipe[1], "EOD\n");
        dprintf(ptc_pipe[1], "splot $data w pm3d\n");

        t += .1;
    } 
}
