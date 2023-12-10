cmake_minimum_required(VERSION 3.0)

project(scheduler)

add_subdirectory(batch_scheduler)

set(sched_hdr
        "scheduler.h"
        "max_weight_scheduler.h"
        "maximal_scheduler.h"
        "qps.h"
        "edge_coloring_batch_scheduler.h"
        # "sb_qps.h"
        #        "miscellaneous.h"
        "scheduler_factory.h"
        "priority_queue.h"
        "qp_sampler.h"
        "rr_sampler.h"
		"circular_generator.h"
	#	 "sliding_window_qps.h"
        # "sliding_window_rr.h"
        )
set(sched_src
        "scheduler.cc"
        "max_weight_scheduler.cc"
        "maximal_scheduler.cc"
        "qps.cc"
        "edge_coloring_batch_scheduler.cc"
        # "sb_qps.cc"
        "priority_queue.cpp"
        "qp_sampler.cpp"
        "rr_sampler.cpp"
	#	 "sliding_window_qps.cpp"
        # "sliding_window_rr.cpp"
        #        "miscellaneous.cc"
        "scheduler_factory.cc")

add_library(scheduler
        ${sched_hdr}
        ${sched_src})
#if (WIN32)
#    else()
#add_dependencies(scheduler
#        json)
#endif()
target_link_libraries(
        scheduler
        batch_scheduler
        ${saber_libraries}
)
