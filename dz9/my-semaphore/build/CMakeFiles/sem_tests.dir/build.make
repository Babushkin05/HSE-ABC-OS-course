# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.31

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
CMAKE_COMMAND = /opt/homebrew/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/build

# Include any dependencies generated for this target.
include CMakeFiles/sem_tests.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/sem_tests.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/sem_tests.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/sem_tests.dir/flags.make

CMakeFiles/sem_tests.dir/codegen:
.PHONY : CMakeFiles/sem_tests.dir/codegen

CMakeFiles/sem_tests.dir/tests/unit.cpp.o: CMakeFiles/sem_tests.dir/flags.make
CMakeFiles/sem_tests.dir/tests/unit.cpp.o: /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/tests/unit.cpp
CMakeFiles/sem_tests.dir/tests/unit.cpp.o: CMakeFiles/sem_tests.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/sem_tests.dir/tests/unit.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/sem_tests.dir/tests/unit.cpp.o -MF CMakeFiles/sem_tests.dir/tests/unit.cpp.o.d -o CMakeFiles/sem_tests.dir/tests/unit.cpp.o -c /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/tests/unit.cpp

CMakeFiles/sem_tests.dir/tests/unit.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/sem_tests.dir/tests/unit.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/tests/unit.cpp > CMakeFiles/sem_tests.dir/tests/unit.cpp.i

CMakeFiles/sem_tests.dir/tests/unit.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/sem_tests.dir/tests/unit.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/tests/unit.cpp -o CMakeFiles/sem_tests.dir/tests/unit.cpp.s

CMakeFiles/sem_tests.dir/tests/stress.cpp.o: CMakeFiles/sem_tests.dir/flags.make
CMakeFiles/sem_tests.dir/tests/stress.cpp.o: /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/tests/stress.cpp
CMakeFiles/sem_tests.dir/tests/stress.cpp.o: CMakeFiles/sem_tests.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/sem_tests.dir/tests/stress.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/sem_tests.dir/tests/stress.cpp.o -MF CMakeFiles/sem_tests.dir/tests/stress.cpp.o.d -o CMakeFiles/sem_tests.dir/tests/stress.cpp.o -c /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/tests/stress.cpp

CMakeFiles/sem_tests.dir/tests/stress.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/sem_tests.dir/tests/stress.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/tests/stress.cpp > CMakeFiles/sem_tests.dir/tests/stress.cpp.i

CMakeFiles/sem_tests.dir/tests/stress.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/sem_tests.dir/tests/stress.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/tests/stress.cpp -o CMakeFiles/sem_tests.dir/tests/stress.cpp.s

# Object files for target sem_tests
sem_tests_OBJECTS = \
"CMakeFiles/sem_tests.dir/tests/unit.cpp.o" \
"CMakeFiles/sem_tests.dir/tests/stress.cpp.o"

# External object files for target sem_tests
sem_tests_EXTERNAL_OBJECTS =

sem_tests: CMakeFiles/sem_tests.dir/tests/unit.cpp.o
sem_tests: CMakeFiles/sem_tests.dir/tests/stress.cpp.o
sem_tests: CMakeFiles/sem_tests.dir/build.make
sem_tests: libsemaphore.a
sem_tests: lib/libgtest_main.a
sem_tests: lib/libgtest.a
sem_tests: CMakeFiles/sem_tests.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable sem_tests"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sem_tests.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/sem_tests.dir/build: sem_tests
.PHONY : CMakeFiles/sem_tests.dir/build

CMakeFiles/sem_tests.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/sem_tests.dir/cmake_clean.cmake
.PHONY : CMakeFiles/sem_tests.dir/clean

CMakeFiles/sem_tests.dir/depend:
	cd /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/build /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/build /Users/vovababuskin/Documents/study/AKOS/dz9/my-semaphore/build/CMakeFiles/sem_tests.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/sem_tests.dir/depend

