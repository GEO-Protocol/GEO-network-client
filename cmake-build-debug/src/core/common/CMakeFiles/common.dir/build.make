# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/denis/Downloads/clion-2016.3.2/bin/cmake/bin/cmake

# The command to remove a file.
RM = /home/denis/Downloads/clion-2016.3.2/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/denis/Projects/client

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/denis/Projects/client/cmake-build-debug

# Include any dependencies generated for this target.
include src/core/common/CMakeFiles/common.dir/depend.make

# Include the progress variables for this target.
include src/core/common/CMakeFiles/common.dir/progress.make

# Include the compile flags for this target's objects.
include src/core/common/CMakeFiles/common.dir/flags.make

src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.o: src/core/common/CMakeFiles/common.dir/flags.make
src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.o: ../src/core/common/NodeUUID.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.o"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/common && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/common.dir/NodeUUID.cpp.o -c /home/denis/Projects/client/src/core/common/NodeUUID.cpp

src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/common.dir/NodeUUID.cpp.i"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/common && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/denis/Projects/client/src/core/common/NodeUUID.cpp > CMakeFiles/common.dir/NodeUUID.cpp.i

src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/common.dir/NodeUUID.cpp.s"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/common && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/denis/Projects/client/src/core/common/NodeUUID.cpp -o CMakeFiles/common.dir/NodeUUID.cpp.s

src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.o.requires:

.PHONY : src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.o.requires

src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.o.provides: src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.o.requires
	$(MAKE) -f src/core/common/CMakeFiles/common.dir/build.make src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.o.provides.build
.PHONY : src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.o.provides

src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.o.provides.build: src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.o


# Object files for target common
common_OBJECTS = \
"CMakeFiles/common.dir/NodeUUID.cpp.o"

# External object files for target common
common_EXTERNAL_OBJECTS =

src/core/common/libcommon.a: src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.o
src/core/common/libcommon.a: src/core/common/CMakeFiles/common.dir/build.make
src/core/common/libcommon.a: src/core/common/CMakeFiles/common.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libcommon.a"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/common && $(CMAKE_COMMAND) -P CMakeFiles/common.dir/cmake_clean_target.cmake
	cd /home/denis/Projects/client/cmake-build-debug/src/core/common && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/common.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/core/common/CMakeFiles/common.dir/build: src/core/common/libcommon.a

.PHONY : src/core/common/CMakeFiles/common.dir/build

src/core/common/CMakeFiles/common.dir/requires: src/core/common/CMakeFiles/common.dir/NodeUUID.cpp.o.requires

.PHONY : src/core/common/CMakeFiles/common.dir/requires

src/core/common/CMakeFiles/common.dir/clean:
	cd /home/denis/Projects/client/cmake-build-debug/src/core/common && $(CMAKE_COMMAND) -P CMakeFiles/common.dir/cmake_clean.cmake
.PHONY : src/core/common/CMakeFiles/common.dir/clean

src/core/common/CMakeFiles/common.dir/depend:
	cd /home/denis/Projects/client/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/denis/Projects/client /home/denis/Projects/client/src/core/common /home/denis/Projects/client/cmake-build-debug /home/denis/Projects/client/cmake-build-debug/src/core/common /home/denis/Projects/client/cmake-build-debug/src/core/common/CMakeFiles/common.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/core/common/CMakeFiles/common.dir/depend

