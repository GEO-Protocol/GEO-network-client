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
include src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/depend.make

# Include the progress variables for this target.
include src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/progress.make

# Include the compile flags for this target's objects.
include src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/flags.make

src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o: src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/flags.make
src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o: ../src/core/db/uuid_map_block_storage/Record.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/db/uuid_map_block_storage && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o -c /home/denis/Projects/client/src/core/db/uuid_map_block_storage/Record.cpp

src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.i"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/db/uuid_map_block_storage && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/denis/Projects/client/src/core/db/uuid_map_block_storage/Record.cpp > CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.i

src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.s"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/db/uuid_map_block_storage && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/denis/Projects/client/src/core/db/uuid_map_block_storage/Record.cpp -o CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.s

src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o.requires:

.PHONY : src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o.requires

src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o.provides: src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o.requires
	$(MAKE) -f src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/build.make src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o.provides.build
.PHONY : src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o.provides

src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o.provides.build: src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o


src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o: src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/flags.make
src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o: ../src/core/db/uuid_map_block_storage/UUIDMapBlockStorage.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/db/uuid_map_block_storage && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o -c /home/denis/Projects/client/src/core/db/uuid_map_block_storage/UUIDMapBlockStorage.cpp

src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.i"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/db/uuid_map_block_storage && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/denis/Projects/client/src/core/db/uuid_map_block_storage/UUIDMapBlockStorage.cpp > CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.i

src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.s"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/db/uuid_map_block_storage && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/denis/Projects/client/src/core/db/uuid_map_block_storage/UUIDMapBlockStorage.cpp -o CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.s

src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o.requires:

.PHONY : src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o.requires

src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o.provides: src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o.requires
	$(MAKE) -f src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/build.make src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o.provides.build
.PHONY : src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o.provides

src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o.provides.build: src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o


# Object files for target db__uuid_map_block_storage
db__uuid_map_block_storage_OBJECTS = \
"CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o" \
"CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o"

# External object files for target db__uuid_map_block_storage
db__uuid_map_block_storage_EXTERNAL_OBJECTS =

src/core/db/uuid_map_block_storage/libdb__uuid_map_block_storage.a: src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o
src/core/db/uuid_map_block_storage/libdb__uuid_map_block_storage.a: src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o
src/core/db/uuid_map_block_storage/libdb__uuid_map_block_storage.a: src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/build.make
src/core/db/uuid_map_block_storage/libdb__uuid_map_block_storage.a: src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libdb__uuid_map_block_storage.a"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/db/uuid_map_block_storage && $(CMAKE_COMMAND) -P CMakeFiles/db__uuid_map_block_storage.dir/cmake_clean_target.cmake
	cd /home/denis/Projects/client/cmake-build-debug/src/core/db/uuid_map_block_storage && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/db__uuid_map_block_storage.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/build: src/core/db/uuid_map_block_storage/libdb__uuid_map_block_storage.a

.PHONY : src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/build

src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/requires: src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/Record.cpp.o.requires
src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/requires: src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/UUIDMapBlockStorage.cpp.o.requires

.PHONY : src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/requires

src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/clean:
	cd /home/denis/Projects/client/cmake-build-debug/src/core/db/uuid_map_block_storage && $(CMAKE_COMMAND) -P CMakeFiles/db__uuid_map_block_storage.dir/cmake_clean.cmake
.PHONY : src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/clean

src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/depend:
	cd /home/denis/Projects/client/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/denis/Projects/client /home/denis/Projects/client/src/core/db/uuid_map_block_storage /home/denis/Projects/client/cmake-build-debug /home/denis/Projects/client/cmake-build-debug/src/core/db/uuid_map_block_storage /home/denis/Projects/client/cmake-build-debug/src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/core/db/uuid_map_block_storage/CMakeFiles/db__uuid_map_block_storage.dir/depend

