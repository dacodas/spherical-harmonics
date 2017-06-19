#include "interactive_prompt.h"

int interactive_prompt()
{
    while (true)
    {
	char* line = readline( "sph-harmonics> ");

	char command[100];
	size_t l, m;
	double scale;

	int n_results = sscanf(line, "%s %d %d %lf", command, &l, &m, &scale);

	if ( n_results != 4 )
	{
	    printf("That's not a valid command!");
	    free(line);
	    continue;
	}

	std::unique_lock<std::mutex> guard(sph_harm::scales_mutex);
	printf("Settings harmonic with l=%d, m=%d to %f\n", l, m, scale);
	sph_harm::scales[l][m] = scale;
	free(line);
    }
    
    return 0;
}
