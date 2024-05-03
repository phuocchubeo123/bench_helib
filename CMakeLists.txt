cmake_minimum_required(VERSION 3.22)
project(BenchmarkHElib)

find_package(helib)

add_executable(bench
    bench.cpp)

target_link_libraries(bench 
    helib)