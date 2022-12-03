cmake_minimum_required(VERSION 3.12)
project(grand-challenge-2)

set(CMAKE_CXX_STANDARD 14)

find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

add_executable(temp src/temp.cpp)

target_link_libraries(temp ${MPI_C_LIBRARIES})