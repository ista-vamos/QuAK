add_library(utils STATIC utils.cpp)
target_include_directories(utils PRIVATE ${SRCDIR})

add_executable(inclusion inclusion.cpp)
target_include_directories(inclusion PRIVATE ${SRCDIR})
target_link_libraries(inclusion PUBLIC quak utils)

add_executable(measure-inclusion measure-inclusion.cpp)
target_include_directories(measure-inclusion PRIVATE ${SRCDIR})
target_link_libraries(measure-inclusion PUBLIC quak utils)



find_package(vamos-buffers QUIET)

# if vamos-buffers were not found
if (NOT vamos-buffers_FOUND)
	message(STATUS "vamos-buffers were not found, looking for them in vamos_DIR=${vamos_DIR}")
	if (vamos_DIR)
		set(vamos-buffers_DIR ${vamos_DIR}/vamos-buffers/cmake/vamos-buffers)
		find_package(vamos-buffers)
	endif()
endif()

if (vamos-buffers_FOUND)
	message(STATUS "Found VAMOS buffers")
	get_filename_component(vamos-buffers_DIR ${vamos-buffers_DIR} ABSOLUTE)

	add_executable(monitor monitor.cpp)
	target_include_directories(monitor PRIVATE ${SRCDIR})
	target_link_libraries(monitor PRIVATE quak utils)
endif()

