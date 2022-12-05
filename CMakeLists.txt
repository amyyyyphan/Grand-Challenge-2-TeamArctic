# NOTE: Every top-level CMakeLists.txt must start with these two lines
cmake_minimum_required(VERSION 3.16)
project(grand-challenge-2)

## Enable C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
set(CMAKE_C_COMPILER "/usr/local/Cellar/llvm/15.0.6/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/local/Cellar/llvm/15.0.6/bin/clang++")
set(OPENMP_LIBRARIES "/usr/local/Cellar/llvm/15.0.6/lib")
set(OPENMP_INCLUDES "/usr/local/Cellar/llvm/15.0.6/include")

## Find dependencies

# OpenMP
find_package(OpenMP REQUIRED)

# MPI
find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

# Boost
set(BOOST_ROOT "" CACHE PATH "Boost build root (useful on Windows)")
option(Boost_USE_STATIC_LIBS
       "Search for static boost libs" OFF)
option(Boost_USE_MULTITHREADED
       "Search for multithreaded boost libs" ON)
option(Boost_USE_STATIC_RUNTIME
       "Search for boost libs linked against static C++ runtime" OFF)
find_package(Boost 1.47.0 REQUIRED COMPONENTS filesystem system)

## Add main project targets

# NOTE: This is completely wrong and will break incremental builds after
#       doing a "git pull" or similar. You should never glob for source
#       files, but instead list them explicitly.
file(GLOB SOURCES "src/*.cpp")
 
add_library(grand-challenge-2 ${SOURCES})
target_include_directories(grand-challenge-2 PUBLIC BUILD_INTERFACE:include)
target_link_libraries(grand-challenge-2
                      PUBLIC
                      OpenMP::OpenMP_CXX
                      Boost::boost 
                      Boost::filesystem
                      Boost::system
                      ${MPI_C_LIBRARIES})

add_executable(manager src/manager.cpp)
add_executable(worker src/worker.cpp)
target_link_libraries(manager PRIVATE grand-challenge-2)
target_link_libraries(worker PRIVATE grand-challenge-2)
