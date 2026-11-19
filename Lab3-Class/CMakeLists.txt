cmake_minimum_required(VERSION 3.20)
project(Lab3_Class)

set(CMAKE_CXX_STANDARD 20)

find_package(MPI REQUIRED)

add_executable(Lab3_Class main.cpp)
target_link_libraries(Lab3_Class PRIVATE MPI::MPI_CXX)
