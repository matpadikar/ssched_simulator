cmake_minimum_required(VERSION 3.0)

project(traffic_generator)


# library for traffic pattern
set(traffic_pattern_hdr
        "traffic_pattern.h"
        "uniform_traffic_pattern.h"
        "log_diagonal_traffic_pattern.h"
        "quasi_diagonal_traffic_pattern.h"
        "diagonal_traffic_pattern.h"
        "traffic_pattern_factory.h")
set(traffic_pattern_src
        "traffic_pattern.cc"
        "uniform_traffic_pattern.cc"
        "log_diagonal_traffic_pattern.cc"
        "quasi_diagonal_traffic_pattern.cc"
        "diagonal_traffic_pattern.cc"
        "traffic_pattern_factory.cc")

add_library(traffic_pattern
        "${traffic_pattern_hdr}"
        "${traffic_pattern_src}")
#if (WIN32)
#    else()
#add_dependencies(traffic_pattern
#        json )
#endif()
target_include_directories (traffic_pattern PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})






