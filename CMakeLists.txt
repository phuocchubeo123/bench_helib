cmake_minimum_required(VERSION 3.22)
project(BenchmarkHElib)

find_package(helib)

add_executable(bench
    bench.cpp)

target_link_libraries(bench 
    helib)

add_executable(dataslot
    tryDataSlot.cpp)

target_link_libraries(dataslot 
    helib)