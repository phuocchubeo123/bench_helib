cmake_minimum_required(VERSION 3.22)
project(BenchmarkHElib)

find_package(helib)


add_library(fimd)
target_include_directories(fimd PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/utils>)

target_link_libraries(fimd
    PUBLIC helib)

set(FIMD_SOURCE_FILES "")

add_subdirectory(utils)

target_sources(fimd PRIVATE ${FIMD_SOURCE_FILES})


message(FIMD_SOURCE_FILES="${FIMD_SOURCE_FILES}")

add_subdirectory(bin)

target_link_libraries(dataslot helib fimd)
target_link_libraries(bench helib fimd)
