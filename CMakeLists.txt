cmake_minimum_required(VERSION 3.0)

project(SSchedSimulator)

set(CMAKE_CXX_STANDARD 11)

# options
option(WITH_TESTS "Build with tests enabled" ON)
option(BUILD_SEPARATE_MODELS "Build each module as a separate library" OFF)
option(ENABLE_LOGGING "Build with logging enabled" ON)
# TODO: HDR is not used anymore, remove it
option(ENABLE_HDRHISTOGRAM "Build with HdrHistogram library enabled" OFF)
option(ENABLE_DEBUG "Build with more verbose output enabled" OFF)

# libraries
set(saber_libraries "")

set(VCPKG_ERR "We only support using vcpkg right now. Please add \"-DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\\scripts\\buildsystems\\vcpkg.cmake\"")

if (NOT CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "${VCPKG_ERR}")
else ()
    get_filename_component(toolchain_file ${CMAKE_TOOLCHAIN_FILE} NAME_WE)
    if (NOT ${toolchain_file} MATCHES "vcpkg")
        message(FATAL_ERROR "${VCPKG_ERR}")
    endif ()

    set(USE_VCPKG ON)
endif ()

if (USE_VCPKG)
    # Use vcpkg
    find_path(JSON_INCLUDE_DIR nlohmann/json.hpp)
    find_path(CACHE2_INCLUDE_DIR catch2/catch_all.hpp)
    find_path(HDR_INCLUDE_DIR hdr/hdr_histogram.h)
    if (ENABLE_LOGGING)
        find_path(SPDLOG_INCLUDE_DIR spdlog/spdlog.h)
        find_package(fmt CONFIG REQUIRED)
        set(FMT_LIBRARY fmt::fmt)
        include_directories(${SPDLOG_INCLUDE_DIR})
        add_definitions(-DUSE_LOGGING)
        list(APPEND saber_libraries ${FMT_LIBRARY})
    endif ()
    #include_directories(${JSON_INCLUDE_DIR} ${CACHE2_INCLUDE_DIR} ${HDR_INCLUDE_DIR})
    include_directories(${JSON_INCLUDE_DIR} ${CACHE2_INCLUDE_DIR})

    #find_library(HDR_LIBRARY hdr_histogram_static)
    find_package(ZLIB REQUIRED)
    set(ZLIB_LIBRARAY ZLIB::ZLIB)
    find_package(LEMON CONFIG REQUIRED)
    set(LEMON_LIBRARY unofficial::lemon::lemon)
    #list(APPEND saber_libraries ${HDR_LIBRARY} ${ZLIB_LIBRARAY} ${LEMON_LIBRARY})
    list(APPEND saber_libraries ${ZLIB_LIBRARAY} ${LEMON_LIBRARY})
else ()
    message(FATAL_ERROR "${VCPKG_ERR}")
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/saber ${CMAKE_CURRENT_SOURCE_DIR}/src)
# Subdirectories
add_subdirectory(src)
#add_subdirectory(test)
#add_subdirectory(example)

#enable_testing ()
#add_subdirectory (test)


# add executable
add_executable(ssched_simulator
        ssched_simulator.cc)

if (ENABLE_DEBUG)
    target_compile_definitions(ssched_simulator PUBLIC -DDEBUG)
endif ()
target_link_libraries(ssched_simulator
        scheduler
        batch_scheduler
        switch
        simulator
        ${sbaer_libraries})
