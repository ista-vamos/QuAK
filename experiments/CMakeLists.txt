cmake_minimum_required(VERSION 3.9)
project(QuAK-experiments)

# OPTION(ENABLE_IPO "Enable interprocedural optimizations" OFF)
# 
# if (ENABLE_IPO)
# 	cmake_policy(SET CMP0069 NEW)
# 	include(CheckIPOSupported)
# 	check_ipo_supported()
# 	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
# endif(ENABLE_IPO)

#enable_testing()

set(SRCDIR ../src)
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
	${SRCDIR}/FORKLIFT/inclusion.cpp
	${SRCDIR}/FORKLIFT/FixpointLoop.cpp
	${SRCDIR}/FORKLIFT/FixpointStem.cpp
	${SRCDIR}/FORKLIFT/ContextOf.cpp
	${SRCDIR}/FORKLIFT/PostContextVariable.cpp
	${SRCDIR}/FORKLIFT/PostTargetVariable.cpp
	${SRCDIR}/FORKLIFT/StateRelation.cpp
	${SRCDIR}/FORKLIFT/TargetOf.cpp
	)

add_executable(inclusion inclusion.cpp)
target_include_directories(inclusion PRIVATE ${SRCDIR})
target_link_libraries(inclusion PRIVATE quak )
