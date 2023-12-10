cmake_minimum_required(VERSION 3.0)

project(test)


set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_TOOLCHAIN_FILE /Users/freeslope/Codes/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(LEMON_DIR /Users/freeslope/Codes/vcpkg/installed/x64-osx/share/lemon)
find_package(LEMON CONFIG REQUIRED)
set(LEMON_LIBRARY unofficial::lemon::lemon)

include_directories(../src)
include_directories(${LEMON_INCLUDE_DIR})

add_subdirectory(../src build)
add_executable(test "static_matrix.cpp")
#target_link_libraries(test matching)
target_link_libraries(test matching ${LEMON_LIBRARY} traffic_pattern)