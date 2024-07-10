cmake_minimum_required(VERSION 3.9)
project(QuAK)

OPTION(ENABLE_SCC_SEARCH_OPT "Enable SCC search optimization in FORKLIFT" OFF)
OPTION(ENABLE_CONTEXT_REDUNDANCY_OPT "Enable context redundancy optimization in FORKLIFT" ON)

if (ENABLE_SCC_SEARCH_OPT)
	add_compile_definitions(INCLUSION_SCC_SEARCH_ACTIVE)
endif()
if (ENABLE_CONTEXT_REDUNDANCY_OPT)
	add_compile_definitions(CONTEXT_REDUNDANCY_ACTIVE)
endif()

# OPTION(ENABLE_IPO "Enable interprocedural optimizations" OFF)
# 
# if (ENABLE_IPO)
# 	cmake_policy(SET CMP0069 NEW)
# 	include(CheckIPOSupported)
# 	check_ipo_supported()
# 	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
# endif(ENABLE_IPO)

#set(CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(SRCDIR src)
get_filename_component(SRCDIR ${SRCDIR} ABSOLUTE)

add_library(quak STATIC
	${SRCDIR}/Automaton.cpp
	${SRCDIR}/Edge.cpp
	${SRCDIR}/Map.cpp
	${SRCDIR}/Set.cpp
	${SRCDIR}/State.cpp
	${SRCDIR}/Symbol.cpp
	${SRCDIR}/Weight.cpp
	${SRCDIR}/Word.cpp
	${SRCDIR}/Parser.cpp
	${SRCDIR}/Monitor.cpp
	${SRCDIR}/RandomAutomaton.cpp
	${SRCDIR}/FORKLIFT/inclusion.cpp
	${SRCDIR}/FORKLIFT/FixpointLoop.cpp
	${SRCDIR}/FORKLIFT/FixpointStem.cpp
	${SRCDIR}/FORKLIFT/ContextOf.cpp
	${SRCDIR}/FORKLIFT/PostContextVariable.cpp
	${SRCDIR}/FORKLIFT/PostTargetVariable.cpp
	${SRCDIR}/FORKLIFT/StateRelation.cpp
	${SRCDIR}/FORKLIFT/TargetOf.cpp
)

OPTION(BUILD_EXPERIMENTS "Enable and build experiments" ON)
add_subdirectory(experiments)

enable_testing()
add_subdirectory(tests)
