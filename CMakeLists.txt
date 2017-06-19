cmake_minimum_required(VERSION 3.6)
project(spherical-harmonics)

add_executable(${PROJECT_NAME}
  gnuplot_interface.cpp
  globals.cpp
  interactive_prompt.cpp
  main.cpp
  )

target_include_directories(${PROJECT_NAME} PUBLIC /usr/include/gsl)
target_link_libraries(${PROJECT_NAME} gsl gslcblas readline pthread)
