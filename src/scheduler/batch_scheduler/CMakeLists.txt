cmake_minimum_required(VERSION 3.0)

set(batch_sched_hdr
        "batch_scheduler.h"
        "sb_qps.h"
        "sliding_window_qps.h"
        "sliding_window_rr.h"
)

set(batch_sched_src
        "batch_scheduler.cc"
        "sb_qps.cc"
        "sliding_window_qps.cpp"
        "sliding_window_rr.cpp"
)

add_library(batch_scheduler
        ${batch_sched_hdr}
        ${batch_sched_src}
)

#if (WIN32)
#    else()
#add_dependencies(scheduler
#        json)
#endif()

target_link_libraries(batch_scheduler
        scheduler
        ${saber_libraries}
)
