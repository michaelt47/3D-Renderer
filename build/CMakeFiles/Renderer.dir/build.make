# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 4.0

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
CMAKE_SOURCE_DIR = "/Users/michael/3D Renderer"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/Users/michael/3D Renderer/build"

# Include any dependencies generated for this target.
include CMakeFiles/Renderer.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/Renderer.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/Renderer.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Renderer.dir/flags.make

CMakeFiles/Renderer.dir/codegen:
.PHONY : CMakeFiles/Renderer.dir/codegen

CMakeFiles/Renderer.dir/main.cpp.o: CMakeFiles/Renderer.dir/flags.make
CMakeFiles/Renderer.dir/main.cpp.o: /Users/michael/3D\ Renderer/main.cpp
CMakeFiles/Renderer.dir/main.cpp.o: CMakeFiles/Renderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir="/Users/michael/3D Renderer/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Renderer.dir/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Renderer.dir/main.cpp.o -MF CMakeFiles/Renderer.dir/main.cpp.o.d -o CMakeFiles/Renderer.dir/main.cpp.o -c "/Users/michael/3D Renderer/main.cpp"

CMakeFiles/Renderer.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/Renderer.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/Users/michael/3D Renderer/main.cpp" > CMakeFiles/Renderer.dir/main.cpp.i

CMakeFiles/Renderer.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/Renderer.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/Users/michael/3D Renderer/main.cpp" -o CMakeFiles/Renderer.dir/main.cpp.s

# Object files for target Renderer
Renderer_OBJECTS = \
"CMakeFiles/Renderer.dir/main.cpp.o"

# External object files for target Renderer
Renderer_EXTERNAL_OBJECTS =

Renderer: CMakeFiles/Renderer.dir/main.cpp.o
Renderer: CMakeFiles/Renderer.dir/build.make
Renderer: /opt/homebrew/Cellar/sdl2/2.32.8/lib/libSDL2.dylib
Renderer: CMakeFiles/Renderer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir="/Users/michael/3D Renderer/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Renderer"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Renderer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Renderer.dir/build: Renderer
.PHONY : CMakeFiles/Renderer.dir/build

CMakeFiles/Renderer.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Renderer.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Renderer.dir/clean

CMakeFiles/Renderer.dir/depend:
	cd "/Users/michael/3D Renderer/build" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/Users/michael/3D Renderer" "/Users/michael/3D Renderer" "/Users/michael/3D Renderer/build" "/Users/michael/3D Renderer/build" "/Users/michael/3D Renderer/build/CMakeFiles/Renderer.dir/DependInfo.cmake" "--color=$(COLOR)"
.PHONY : CMakeFiles/Renderer.dir/depend

