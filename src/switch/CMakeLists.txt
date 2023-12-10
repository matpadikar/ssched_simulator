cmake_minimum_required(VERSION 3.0)

project(switch)

# library
set(switch_hdr
        "switch.h"
        "iq_switch.h"
        "iq_switch_factory.h")
set(switch_src
        "switch.cc"
        "iq_switch.cc"
        "iq_switch_factory.cc")
add_library(switch
        ${switch_hdr}
        ${switch_src})
#if (WIN32)
#    else()
#add_dependencies(switch
#        json)
#endif()
target_link_libraries(switch
        scheduler
        batch_scheduler
        ${saber_libraries})

