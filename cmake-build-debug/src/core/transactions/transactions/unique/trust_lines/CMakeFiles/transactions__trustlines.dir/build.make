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
include src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/depend.make

# Include the progress variables for this target.
include src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/progress.make

# Include the compile flags for this target's objects.
include src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/flags.make

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/flags.make
src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o: ../src/core/transactions/transactions/unique/trust_lines/TrustLineTransaction.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o -c /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/TrustLineTransaction.cpp

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.i"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/TrustLineTransaction.cpp > CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.i

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.s"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/TrustLineTransaction.cpp -o CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.s

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o.requires:

.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o.requires

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o.provides: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o.requires
	$(MAKE) -f src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/build.make src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o.provides.build
.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o.provides

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o.provides.build: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o


src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/flags.make
src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o: ../src/core/transactions/transactions/unique/trust_lines/OpenTrustLineTransaction.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o -c /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/OpenTrustLineTransaction.cpp

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.i"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/OpenTrustLineTransaction.cpp > CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.i

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.s"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/OpenTrustLineTransaction.cpp -o CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.s

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o.requires:

.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o.requires

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o.provides: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o.requires
	$(MAKE) -f src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/build.make src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o.provides.build
.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o.provides

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o.provides.build: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o


src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/flags.make
src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o: ../src/core/transactions/transactions/unique/trust_lines/AcceptTrustLineTransaction.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o -c /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/AcceptTrustLineTransaction.cpp

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.i"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/AcceptTrustLineTransaction.cpp > CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.i

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.s"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/AcceptTrustLineTransaction.cpp -o CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.s

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o.requires:

.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o.requires

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o.provides: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o.requires
	$(MAKE) -f src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/build.make src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o.provides.build
.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o.provides

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o.provides.build: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o


src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/flags.make
src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o: ../src/core/transactions/transactions/unique/trust_lines/CloseTrustLineTransaction.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o -c /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/CloseTrustLineTransaction.cpp

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.i"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/CloseTrustLineTransaction.cpp > CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.i

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.s"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/CloseTrustLineTransaction.cpp -o CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.s

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o.requires:

.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o.requires

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o.provides: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o.requires
	$(MAKE) -f src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/build.make src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o.provides.build
.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o.provides

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o.provides.build: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o


src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/flags.make
src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o: ../src/core/transactions/transactions/unique/trust_lines/RejectTrustLineTransaction.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o -c /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/RejectTrustLineTransaction.cpp

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.i"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/RejectTrustLineTransaction.cpp > CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.i

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.s"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/RejectTrustLineTransaction.cpp -o CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.s

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o.requires:

.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o.requires

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o.provides: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o.requires
	$(MAKE) -f src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/build.make src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o.provides.build
.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o.provides

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o.provides.build: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o


src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/flags.make
src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o: ../src/core/transactions/transactions/unique/trust_lines/SetTrustLineTransaction.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o -c /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/SetTrustLineTransaction.cpp

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.i"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/SetTrustLineTransaction.cpp > CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.i

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.s"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/SetTrustLineTransaction.cpp -o CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.s

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o.requires:

.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o.requires

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o.provides: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o.requires
	$(MAKE) -f src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/build.make src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o.provides.build
.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o.provides

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o.provides.build: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o


src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/flags.make
src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o: ../src/core/transactions/transactions/unique/trust_lines/UpdateTrustLineTransaction.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o -c /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/UpdateTrustLineTransaction.cpp

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.i"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/UpdateTrustLineTransaction.cpp > CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.i

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.s"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines/UpdateTrustLineTransaction.cpp -o CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.s

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o.requires:

.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o.requires

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o.provides: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o.requires
	$(MAKE) -f src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/build.make src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o.provides.build
.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o.provides

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o.provides.build: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o


# Object files for target transactions__trustlines
transactions__trustlines_OBJECTS = \
"CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o" \
"CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o" \
"CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o" \
"CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o" \
"CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o" \
"CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o" \
"CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o"

# External object files for target transactions__trustlines
transactions__trustlines_EXTERNAL_OBJECTS =

src/core/transactions/transactions/unique/trust_lines/libtransactions__trustlines.a: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o
src/core/transactions/transactions/unique/trust_lines/libtransactions__trustlines.a: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o
src/core/transactions/transactions/unique/trust_lines/libtransactions__trustlines.a: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o
src/core/transactions/transactions/unique/trust_lines/libtransactions__trustlines.a: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o
src/core/transactions/transactions/unique/trust_lines/libtransactions__trustlines.a: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o
src/core/transactions/transactions/unique/trust_lines/libtransactions__trustlines.a: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o
src/core/transactions/transactions/unique/trust_lines/libtransactions__trustlines.a: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o
src/core/transactions/transactions/unique/trust_lines/libtransactions__trustlines.a: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/build.make
src/core/transactions/transactions/unique/trust_lines/libtransactions__trustlines.a: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Linking CXX static library libtransactions__trustlines.a"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && $(CMAKE_COMMAND) -P CMakeFiles/transactions__trustlines.dir/cmake_clean_target.cmake
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/transactions__trustlines.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/build: src/core/transactions/transactions/unique/trust_lines/libtransactions__trustlines.a

.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/build

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/requires: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/TrustLineTransaction.cpp.o.requires
src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/requires: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/OpenTrustLineTransaction.cpp.o.requires
src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/requires: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/AcceptTrustLineTransaction.cpp.o.requires
src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/requires: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/CloseTrustLineTransaction.cpp.o.requires
src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/requires: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/RejectTrustLineTransaction.cpp.o.requires
src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/requires: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/SetTrustLineTransaction.cpp.o.requires
src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/requires: src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/UpdateTrustLineTransaction.cpp.o.requires

.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/requires

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/clean:
	cd /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines && $(CMAKE_COMMAND) -P CMakeFiles/transactions__trustlines.dir/cmake_clean.cmake
.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/clean

src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/depend:
	cd /home/denis/Projects/client/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/denis/Projects/client /home/denis/Projects/client/src/core/transactions/transactions/unique/trust_lines /home/denis/Projects/client/cmake-build-debug /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines /home/denis/Projects/client/cmake-build-debug/src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/core/transactions/transactions/unique/trust_lines/CMakeFiles/transactions__trustlines.dir/depend

