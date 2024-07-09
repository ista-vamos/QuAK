add_library(utils STATIC utils.cpp)
target_include_directories(utils PRIVATE ${SRCDIR})

add_executable(inclusion inclusion.cpp)
target_include_directories(inclusion PRIVATE ${SRCDIR})
target_link_libraries(inclusion PRIVATE quak utils)

find_package(vamos-buffers)

if (vamos-buffers_FOUND)
	message(STATUS "Found VAMOS buffers")
	get_filename_component(vamos-buffers_DIR ${vamos-buffers_DIR} ABSOLUTE)

	add_executable(monitor monitor.cpp)
	target_include_directories(monitor PRIVATE ${SRCDIR})
	target_link_libraries(monitor PRIVATE quak utils)
endif()

