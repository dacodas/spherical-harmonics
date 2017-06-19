#include <thread>
#include "gnuplot_interface.h"
#include "interactive_prompt.h"

int main()
{
    std::thread gnuplot(gnuplot_task);
    std::thread prompt(interactive_prompt);

    gnuplot.join();
    prompt.join();
    
    return 0;
}
