# Tests for ssched_simulator
cmake_minimum_required(VERSION 3.0)

project(SSchedSimulatorExample)


# Examples for this project
set(saber_examples "")

## traffic generator
add_executable(traffic_generator_example
        "traffic_generator_example.cc")
add_dependencies(traffic_generator_example
        json
        injection_model
        traffic_pattern)

target_link_libraries(traffic_generator_example
        injection_model
        traffic_pattern)
list(APPEND saber_examples
        traffic_generator_example)


## packet
add_executable(packet_example
        "packet_example.cc")
list(APPEND saber_examples
        packet_example)

## timer/channel/stats
add_executable(util_example
        "util_example.cc")
target_link_libraries(util_example
        ${saber_libraries})
list(APPEND saber_examples
        util_example)

## scheduler
add_executable(sched_example
        "scheduler_example.cc")
target_link_libraries(sched_example
        scheduler
        batch_scheduler
        switch
        ${sbaer_libraries})
list(APPEND saber_example
        sched_example)

## switch
add_executable(switch_example
        "switch_example.cc")
target_link_libraries(switch_example
        scheduler
        batch_scheduler
        switch
        ${sbaer_libraries})
list(APPEND saber_example
        switch_example)


# simulator
add_executable(sim_example
        "simulator_example.cc")
target_link_libraries(sim_example
        scheduler
        batch_scheduler
        switch
        simulator
        ${sbaer_libraries})
list(APPEND saber_example
        sim_example)


# Add to customized target example
# !! Still do not figure out a better to avoid re-compiliation
add_custom_target(
        example
        COMMAND $<TARGET_FILE:traffic_generator_example>
        COMMAND $<TARGET_FILE:packet_example>
        COMMAND $<TARGET_FILE:util_example>
        COMMAND $<TARGET_FILE:sched_example>
        COMMAND $<TARGET_FILE:switch_example>
        COMMAND $<TARGET_FILE:sim_example>
        DEPENDS ${saber_examples}
)