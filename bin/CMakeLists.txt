add_executable(bench
    ../bench.cpp)

target_link_libraries(bench 
    helib)

add_executable(dataslot
    ../tryDataSlot.cpp)

target_link_libraries(dataslot 
    helib)

add_executable(find_params
    ../find_params.cpp)