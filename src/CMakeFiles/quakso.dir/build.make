# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /snap/cmake/1417/bin/cmake

# The command to remove a file.
RM = /snap/cmake/1417/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ege/Desktop/repos/QuAK

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ege/Desktop/repos/QuAK

# Include any dependencies generated for this target.
include src/CMakeFiles/quakso.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/quakso.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/quakso.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/quakso.dir/flags.make

src/CMakeFiles/quakso.dir/sharedlib.cpp.o: src/CMakeFiles/quakso.dir/flags.make
src/CMakeFiles/quakso.dir/sharedlib.cpp.o: src/sharedlib.cpp
src/CMakeFiles/quakso.dir/sharedlib.cpp.o: src/CMakeFiles/quakso.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/ege/Desktop/repos/QuAK/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/quakso.dir/sharedlib.cpp.o"
	cd /home/ege/Desktop/repos/QuAK/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/quakso.dir/sharedlib.cpp.o -MF CMakeFiles/quakso.dir/sharedlib.cpp.o.d -o CMakeFiles/quakso.dir/sharedlib.cpp.o -c /home/ege/Desktop/repos/QuAK/src/sharedlib.cpp

src/CMakeFiles/quakso.dir/sharedlib.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/quakso.dir/sharedlib.cpp.i"
	cd /home/ege/Desktop/repos/QuAK/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ege/Desktop/repos/QuAK/src/sharedlib.cpp > CMakeFiles/quakso.dir/sharedlib.cpp.i

src/CMakeFiles/quakso.dir/sharedlib.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/quakso.dir/sharedlib.cpp.s"
	cd /home/ege/Desktop/repos/QuAK/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ege/Desktop/repos/QuAK/src/sharedlib.cpp -o CMakeFiles/quakso.dir/sharedlib.cpp.s

# Object files for target quakso
quakso_OBJECTS = \
"CMakeFiles/quakso.dir/sharedlib.cpp.o"

# External object files for target quakso
quakso_EXTERNAL_OBJECTS =

src/libquak.so: src/CMakeFiles/quakso.dir/sharedlib.cpp.o
src/libquak.so: src/CMakeFiles/quakso.dir/build.make
src/libquak.so: src/libquak.a
src/libquak.so: src/CMakeFiles/quakso.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/ege/Desktop/repos/QuAK/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library libquak.so"
	cd /home/ege/Desktop/repos/QuAK/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/quakso.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/quakso.dir/build: src/libquak.so
.PHONY : src/CMakeFiles/quakso.dir/build

src/CMakeFiles/quakso.dir/clean:
	cd /home/ege/Desktop/repos/QuAK/src && $(CMAKE_COMMAND) -P CMakeFiles/quakso.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/quakso.dir/clean

src/CMakeFiles/quakso.dir/depend:
	cd /home/ege/Desktop/repos/QuAK && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ege/Desktop/repos/QuAK /home/ege/Desktop/repos/QuAK/src /home/ege/Desktop/repos/QuAK /home/ege/Desktop/repos/QuAK/src /home/ege/Desktop/repos/QuAK/src/CMakeFiles/quakso.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : src/CMakeFiles/quakso.dir/depend
