cmake_minimum_required(VERSION 2.8)

project (grand-challenge-2)

find_package(MPI)

SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpicxx)

SET(OPENMP_CXX_FLAGS "-O3 -fopenmp")
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${OPENMP_CXX_FLAGS}" )

# app targets
# add_executable(temp src/temp.cpp)

add_executable(manager src/manager.cpp)
add_executable(worker src/worker.cpp)