cmake_minimum_required(VERSION 3.12)
project(grand-challenge-2)

set(CMAKE_CXX_STANDARD 14)

find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

add_executable(manager src/manager.cpp)
add_executable(worker src/worker.cpp)

target_link_libraries(manager ${MPI_C_LIBRARIES})
target_link_libraries(worker ${MPI_C_LIBRARIES})