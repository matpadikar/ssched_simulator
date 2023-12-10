project(matching)

set(saber_libraries "")
set(CMAKE_BUILD_TYPE DEBUG)
set(matching_headers
	"qp_sampler.h" 
	"sb_qps.h"
	"mwm.hpp" 
	"double_max.h" 
	"availmap.hpp"
)

set(matching_sources
	"qp_sampler.cpp"
	"sb_qps.cc"
	"double_max.cc"
)

add_subdirectory(serena)
add_subdirectory(traffic_generator)
message(${matching_headers})
add_library(matching ${matching_sources} ${matching_headers})