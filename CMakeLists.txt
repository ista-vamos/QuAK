cmake_minimum_required(VERSION 3.9)
project(QuAK)

OPTION(ENABLE_SCC_SEARCH_OPT "Enable SCC search optimization in FORKLIFT" ON)
OPTION(ENABLE_CONTEXT_REDUNDANCY_OPT "Enable context redundancy optimization in FORKLIFT" OFF)

# comparing floats for == is unreliable, we assume they are equal
# if their absolute difference is less than this constant
add_compile_definitions(WEIGHT_EQ_EPSILON=10e-6)

if (ENABLE_SCC_SEARCH_OPT)
	add_compile_definitions(INCLUSION_SCC_SEARCH_ACTIVE)
endif()
if (ENABLE_CONTEXT_REDUNDANCY_OPT)
	add_compile_definitions(CONTEXT_REDUNDANCY_ACTIVE)
endif()

OPTION(ENABLE_IPO "Enable interprocedural optimizations" OFF)

if (ENABLE_IPO)
	cmake_policy(SET CMP0069 NEW)
	include(CheckIPOSupported)
	check_ipo_supported()
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif(ENABLE_IPO)

#set(CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src)

add_executable(quakbin src/quak-main.cpp)
target_include_directories(quakbin PRIVATE ${SRCDIR})
target_link_libraries(quakbin PUBLIC quak quak-private)

set_target_properties(quakbin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src" OUTPUT_NAME quak-main)