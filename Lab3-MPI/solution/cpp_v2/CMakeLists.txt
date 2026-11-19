cmake_minimum_required(VERSION 3.20)
project(cpp_v2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(MPI REQUIRED)

include_directories(.)

add_executable(v0 main_v0.cpp utils.cpp)
add_executable(v1 main_v1.cpp utils.cpp)
add_executable(v1_1 main_v1.1.cpp utils.cpp)
add_executable(v2 main_v2.cpp utils.cpp)
add_executable(v3 main_v3.cpp utils.cpp)

foreach(target v0 v1 v1_1 v2 v3)
    target_link_libraries(${target} PRIVATE MPI::MPI_CXX)
endforeach()
