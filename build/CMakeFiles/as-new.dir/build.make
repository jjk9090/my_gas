# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jjk/project/my_gas

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jjk/project/my_gas/build

# Include any dependencies generated for this target.
include CMakeFiles/as-new.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/as-new.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/as-new.dir/flags.make

CMakeFiles/as-new.dir/as.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/as.c.o: ../as.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/as-new.dir/as.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/as.c.o   -c /home/jjk/project/my_gas/as.c

CMakeFiles/as-new.dir/as.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/as.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/as.c > CMakeFiles/as-new.dir/as.c.i

CMakeFiles/as-new.dir/as.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/as.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/as.c -o CMakeFiles/as-new.dir/as.c.s

CMakeFiles/as-new.dir/src/app.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/app.c.o: ../src/app.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/as-new.dir/src/app.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/app.c.o   -c /home/jjk/project/my_gas/src/app.c

CMakeFiles/as-new.dir/src/app.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/app.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/app.c > CMakeFiles/as-new.dir/src/app.c.i

CMakeFiles/as-new.dir/src/app.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/app.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/app.c -o CMakeFiles/as-new.dir/src/app.c.s

CMakeFiles/as-new.dir/src/atof-generic.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/atof-generic.c.o: ../src/atof-generic.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/as-new.dir/src/atof-generic.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/atof-generic.c.o   -c /home/jjk/project/my_gas/src/atof-generic.c

CMakeFiles/as-new.dir/src/atof-generic.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/atof-generic.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/atof-generic.c > CMakeFiles/as-new.dir/src/atof-generic.c.i

CMakeFiles/as-new.dir/src/atof-generic.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/atof-generic.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/atof-generic.c -o CMakeFiles/as-new.dir/src/atof-generic.c.s

CMakeFiles/as-new.dir/src/bfd.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/bfd.c.o: ../src/bfd.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/as-new.dir/src/bfd.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/bfd.c.o   -c /home/jjk/project/my_gas/src/bfd.c

CMakeFiles/as-new.dir/src/bfd.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/bfd.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/bfd.c > CMakeFiles/as-new.dir/src/bfd.c.i

CMakeFiles/as-new.dir/src/bfd.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/bfd.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/bfd.c -o CMakeFiles/as-new.dir/src/bfd.c.s

CMakeFiles/as-new.dir/src/cond.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/cond.c.o: ../src/cond.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/as-new.dir/src/cond.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/cond.c.o   -c /home/jjk/project/my_gas/src/cond.c

CMakeFiles/as-new.dir/src/cond.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/cond.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/cond.c > CMakeFiles/as-new.dir/src/cond.c.i

CMakeFiles/as-new.dir/src/cond.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/cond.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/cond.c -o CMakeFiles/as-new.dir/src/cond.c.s

CMakeFiles/as-new.dir/src/depend.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/depend.c.o: ../src/depend.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object CMakeFiles/as-new.dir/src/depend.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/depend.c.o   -c /home/jjk/project/my_gas/src/depend.c

CMakeFiles/as-new.dir/src/depend.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/depend.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/depend.c > CMakeFiles/as-new.dir/src/depend.c.i

CMakeFiles/as-new.dir/src/depend.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/depend.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/depend.c -o CMakeFiles/as-new.dir/src/depend.c.s

CMakeFiles/as-new.dir/src/ehopt.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/ehopt.c.o: ../src/ehopt.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building C object CMakeFiles/as-new.dir/src/ehopt.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/ehopt.c.o   -c /home/jjk/project/my_gas/src/ehopt.c

CMakeFiles/as-new.dir/src/ehopt.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/ehopt.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/ehopt.c > CMakeFiles/as-new.dir/src/ehopt.c.i

CMakeFiles/as-new.dir/src/ehopt.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/ehopt.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/ehopt.c -o CMakeFiles/as-new.dir/src/ehopt.c.s

CMakeFiles/as-new.dir/src/expr.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/expr.c.o: ../src/expr.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building C object CMakeFiles/as-new.dir/src/expr.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/expr.c.o   -c /home/jjk/project/my_gas/src/expr.c

CMakeFiles/as-new.dir/src/expr.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/expr.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/expr.c > CMakeFiles/as-new.dir/src/expr.c.i

CMakeFiles/as-new.dir/src/expr.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/expr.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/expr.c -o CMakeFiles/as-new.dir/src/expr.c.s

CMakeFiles/as-new.dir/src/flonum-copy.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/flonum-copy.c.o: ../src/flonum-copy.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building C object CMakeFiles/as-new.dir/src/flonum-copy.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/flonum-copy.c.o   -c /home/jjk/project/my_gas/src/flonum-copy.c

CMakeFiles/as-new.dir/src/flonum-copy.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/flonum-copy.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/flonum-copy.c > CMakeFiles/as-new.dir/src/flonum-copy.c.i

CMakeFiles/as-new.dir/src/flonum-copy.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/flonum-copy.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/flonum-copy.c -o CMakeFiles/as-new.dir/src/flonum-copy.c.s

CMakeFiles/as-new.dir/src/flonum-konst.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/flonum-konst.c.o: ../src/flonum-konst.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building C object CMakeFiles/as-new.dir/src/flonum-konst.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/flonum-konst.c.o   -c /home/jjk/project/my_gas/src/flonum-konst.c

CMakeFiles/as-new.dir/src/flonum-konst.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/flonum-konst.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/flonum-konst.c > CMakeFiles/as-new.dir/src/flonum-konst.c.i

CMakeFiles/as-new.dir/src/flonum-konst.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/flonum-konst.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/flonum-konst.c -o CMakeFiles/as-new.dir/src/flonum-konst.c.s

CMakeFiles/as-new.dir/src/flonum-mult.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/flonum-mult.c.o: ../src/flonum-mult.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building C object CMakeFiles/as-new.dir/src/flonum-mult.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/flonum-mult.c.o   -c /home/jjk/project/my_gas/src/flonum-mult.c

CMakeFiles/as-new.dir/src/flonum-mult.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/flonum-mult.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/flonum-mult.c > CMakeFiles/as-new.dir/src/flonum-mult.c.i

CMakeFiles/as-new.dir/src/flonum-mult.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/flonum-mult.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/flonum-mult.c -o CMakeFiles/as-new.dir/src/flonum-mult.c.s

CMakeFiles/as-new.dir/src/frags.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/frags.c.o: ../src/frags.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building C object CMakeFiles/as-new.dir/src/frags.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/frags.c.o   -c /home/jjk/project/my_gas/src/frags.c

CMakeFiles/as-new.dir/src/frags.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/frags.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/frags.c > CMakeFiles/as-new.dir/src/frags.c.i

CMakeFiles/as-new.dir/src/frags.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/frags.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/frags.c -o CMakeFiles/as-new.dir/src/frags.c.s

CMakeFiles/as-new.dir/src/hash.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/hash.c.o: ../src/hash.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Building C object CMakeFiles/as-new.dir/src/hash.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/hash.c.o   -c /home/jjk/project/my_gas/src/hash.c

CMakeFiles/as-new.dir/src/hash.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/hash.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/hash.c > CMakeFiles/as-new.dir/src/hash.c.i

CMakeFiles/as-new.dir/src/hash.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/hash.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/hash.c -o CMakeFiles/as-new.dir/src/hash.c.s

CMakeFiles/as-new.dir/src/input-file.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/input-file.c.o: ../src/input-file.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Building C object CMakeFiles/as-new.dir/src/input-file.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/input-file.c.o   -c /home/jjk/project/my_gas/src/input-file.c

CMakeFiles/as-new.dir/src/input-file.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/input-file.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/input-file.c > CMakeFiles/as-new.dir/src/input-file.c.i

CMakeFiles/as-new.dir/src/input-file.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/input-file.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/input-file.c -o CMakeFiles/as-new.dir/src/input-file.c.s

CMakeFiles/as-new.dir/src/input-scrub.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/input-scrub.c.o: ../src/input-scrub.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_15) "Building C object CMakeFiles/as-new.dir/src/input-scrub.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/input-scrub.c.o   -c /home/jjk/project/my_gas/src/input-scrub.c

CMakeFiles/as-new.dir/src/input-scrub.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/input-scrub.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/input-scrub.c > CMakeFiles/as-new.dir/src/input-scrub.c.i

CMakeFiles/as-new.dir/src/input-scrub.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/input-scrub.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/input-scrub.c -o CMakeFiles/as-new.dir/src/input-scrub.c.s

CMakeFiles/as-new.dir/src/listing.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/listing.c.o: ../src/listing.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_16) "Building C object CMakeFiles/as-new.dir/src/listing.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/listing.c.o   -c /home/jjk/project/my_gas/src/listing.c

CMakeFiles/as-new.dir/src/listing.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/listing.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/listing.c > CMakeFiles/as-new.dir/src/listing.c.i

CMakeFiles/as-new.dir/src/listing.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/listing.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/listing.c -o CMakeFiles/as-new.dir/src/listing.c.s

CMakeFiles/as-new.dir/src/macro.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/macro.c.o: ../src/macro.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_17) "Building C object CMakeFiles/as-new.dir/src/macro.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/macro.c.o   -c /home/jjk/project/my_gas/src/macro.c

CMakeFiles/as-new.dir/src/macro.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/macro.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/macro.c > CMakeFiles/as-new.dir/src/macro.c.i

CMakeFiles/as-new.dir/src/macro.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/macro.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/macro.c -o CMakeFiles/as-new.dir/src/macro.c.s

CMakeFiles/as-new.dir/src/messages.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/messages.c.o: ../src/messages.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_18) "Building C object CMakeFiles/as-new.dir/src/messages.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/messages.c.o   -c /home/jjk/project/my_gas/src/messages.c

CMakeFiles/as-new.dir/src/messages.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/messages.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/messages.c > CMakeFiles/as-new.dir/src/messages.c.i

CMakeFiles/as-new.dir/src/messages.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/messages.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/messages.c -o CMakeFiles/as-new.dir/src/messages.c.s

CMakeFiles/as-new.dir/src/output-file.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/output-file.c.o: ../src/output-file.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_19) "Building C object CMakeFiles/as-new.dir/src/output-file.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/output-file.c.o   -c /home/jjk/project/my_gas/src/output-file.c

CMakeFiles/as-new.dir/src/output-file.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/output-file.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/output-file.c > CMakeFiles/as-new.dir/src/output-file.c.i

CMakeFiles/as-new.dir/src/output-file.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/output-file.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/output-file.c -o CMakeFiles/as-new.dir/src/output-file.c.s

CMakeFiles/as-new.dir/src/read.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/read.c.o: ../src/read.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_20) "Building C object CMakeFiles/as-new.dir/src/read.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/read.c.o   -c /home/jjk/project/my_gas/src/read.c

CMakeFiles/as-new.dir/src/read.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/read.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/read.c > CMakeFiles/as-new.dir/src/read.c.i

CMakeFiles/as-new.dir/src/read.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/read.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/read.c -o CMakeFiles/as-new.dir/src/read.c.s

CMakeFiles/as-new.dir/src/reloc.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/reloc.c.o: ../src/reloc.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_21) "Building C object CMakeFiles/as-new.dir/src/reloc.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/reloc.c.o   -c /home/jjk/project/my_gas/src/reloc.c

CMakeFiles/as-new.dir/src/reloc.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/reloc.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/reloc.c > CMakeFiles/as-new.dir/src/reloc.c.i

CMakeFiles/as-new.dir/src/reloc.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/reloc.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/reloc.c -o CMakeFiles/as-new.dir/src/reloc.c.s

CMakeFiles/as-new.dir/src/sb.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/sb.c.o: ../src/sb.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_22) "Building C object CMakeFiles/as-new.dir/src/sb.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/sb.c.o   -c /home/jjk/project/my_gas/src/sb.c

CMakeFiles/as-new.dir/src/sb.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/sb.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/sb.c > CMakeFiles/as-new.dir/src/sb.c.i

CMakeFiles/as-new.dir/src/sb.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/sb.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/sb.c -o CMakeFiles/as-new.dir/src/sb.c.s

CMakeFiles/as-new.dir/src/subsegs.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/subsegs.c.o: ../src/subsegs.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_23) "Building C object CMakeFiles/as-new.dir/src/subsegs.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/subsegs.c.o   -c /home/jjk/project/my_gas/src/subsegs.c

CMakeFiles/as-new.dir/src/subsegs.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/subsegs.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/subsegs.c > CMakeFiles/as-new.dir/src/subsegs.c.i

CMakeFiles/as-new.dir/src/subsegs.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/subsegs.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/subsegs.c -o CMakeFiles/as-new.dir/src/subsegs.c.s

CMakeFiles/as-new.dir/src/symbols.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/symbols.c.o: ../src/symbols.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_24) "Building C object CMakeFiles/as-new.dir/src/symbols.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/symbols.c.o   -c /home/jjk/project/my_gas/src/symbols.c

CMakeFiles/as-new.dir/src/symbols.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/symbols.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/symbols.c > CMakeFiles/as-new.dir/src/symbols.c.i

CMakeFiles/as-new.dir/src/symbols.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/symbols.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/symbols.c -o CMakeFiles/as-new.dir/src/symbols.c.s

CMakeFiles/as-new.dir/src/write.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/src/write.c.o: ../src/write.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_25) "Building C object CMakeFiles/as-new.dir/src/write.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/src/write.c.o   -c /home/jjk/project/my_gas/src/write.c

CMakeFiles/as-new.dir/src/write.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/src/write.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/src/write.c > CMakeFiles/as-new.dir/src/write.c.i

CMakeFiles/as-new.dir/src/write.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/src/write.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/src/write.c -o CMakeFiles/as-new.dir/src/write.c.s

CMakeFiles/as-new.dir/config/atof-ieee.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/config/atof-ieee.c.o: ../config/atof-ieee.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_26) "Building C object CMakeFiles/as-new.dir/config/atof-ieee.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/config/atof-ieee.c.o   -c /home/jjk/project/my_gas/config/atof-ieee.c

CMakeFiles/as-new.dir/config/atof-ieee.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/config/atof-ieee.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/config/atof-ieee.c > CMakeFiles/as-new.dir/config/atof-ieee.c.i

CMakeFiles/as-new.dir/config/atof-ieee.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/config/atof-ieee.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/config/atof-ieee.c -o CMakeFiles/as-new.dir/config/atof-ieee.c.s

CMakeFiles/as-new.dir/config/obj-elf.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/config/obj-elf.c.o: ../config/obj-elf.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_27) "Building C object CMakeFiles/as-new.dir/config/obj-elf.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/config/obj-elf.c.o   -c /home/jjk/project/my_gas/config/obj-elf.c

CMakeFiles/as-new.dir/config/obj-elf.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/config/obj-elf.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/config/obj-elf.c > CMakeFiles/as-new.dir/config/obj-elf.c.i

CMakeFiles/as-new.dir/config/obj-elf.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/config/obj-elf.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/config/obj-elf.c -o CMakeFiles/as-new.dir/config/obj-elf.c.s

CMakeFiles/as-new.dir/config/tc-arm.c.o: CMakeFiles/as-new.dir/flags.make
CMakeFiles/as-new.dir/config/tc-arm.c.o: ../config/tc-arm.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_28) "Building C object CMakeFiles/as-new.dir/config/tc-arm.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/as-new.dir/config/tc-arm.c.o   -c /home/jjk/project/my_gas/config/tc-arm.c

CMakeFiles/as-new.dir/config/tc-arm.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/as-new.dir/config/tc-arm.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jjk/project/my_gas/config/tc-arm.c > CMakeFiles/as-new.dir/config/tc-arm.c.i

CMakeFiles/as-new.dir/config/tc-arm.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/as-new.dir/config/tc-arm.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jjk/project/my_gas/config/tc-arm.c -o CMakeFiles/as-new.dir/config/tc-arm.c.s

# Object files for target as-new
as__new_OBJECTS = \
"CMakeFiles/as-new.dir/as.c.o" \
"CMakeFiles/as-new.dir/src/app.c.o" \
"CMakeFiles/as-new.dir/src/atof-generic.c.o" \
"CMakeFiles/as-new.dir/src/bfd.c.o" \
"CMakeFiles/as-new.dir/src/cond.c.o" \
"CMakeFiles/as-new.dir/src/depend.c.o" \
"CMakeFiles/as-new.dir/src/ehopt.c.o" \
"CMakeFiles/as-new.dir/src/expr.c.o" \
"CMakeFiles/as-new.dir/src/flonum-copy.c.o" \
"CMakeFiles/as-new.dir/src/flonum-konst.c.o" \
"CMakeFiles/as-new.dir/src/flonum-mult.c.o" \
"CMakeFiles/as-new.dir/src/frags.c.o" \
"CMakeFiles/as-new.dir/src/hash.c.o" \
"CMakeFiles/as-new.dir/src/input-file.c.o" \
"CMakeFiles/as-new.dir/src/input-scrub.c.o" \
"CMakeFiles/as-new.dir/src/listing.c.o" \
"CMakeFiles/as-new.dir/src/macro.c.o" \
"CMakeFiles/as-new.dir/src/messages.c.o" \
"CMakeFiles/as-new.dir/src/output-file.c.o" \
"CMakeFiles/as-new.dir/src/read.c.o" \
"CMakeFiles/as-new.dir/src/reloc.c.o" \
"CMakeFiles/as-new.dir/src/sb.c.o" \
"CMakeFiles/as-new.dir/src/subsegs.c.o" \
"CMakeFiles/as-new.dir/src/symbols.c.o" \
"CMakeFiles/as-new.dir/src/write.c.o" \
"CMakeFiles/as-new.dir/config/atof-ieee.c.o" \
"CMakeFiles/as-new.dir/config/obj-elf.c.o" \
"CMakeFiles/as-new.dir/config/tc-arm.c.o"

# External object files for target as-new
as__new_EXTERNAL_OBJECTS =

../as-new: CMakeFiles/as-new.dir/as.c.o
../as-new: CMakeFiles/as-new.dir/src/app.c.o
../as-new: CMakeFiles/as-new.dir/src/atof-generic.c.o
../as-new: CMakeFiles/as-new.dir/src/bfd.c.o
../as-new: CMakeFiles/as-new.dir/src/cond.c.o
../as-new: CMakeFiles/as-new.dir/src/depend.c.o
../as-new: CMakeFiles/as-new.dir/src/ehopt.c.o
../as-new: CMakeFiles/as-new.dir/src/expr.c.o
../as-new: CMakeFiles/as-new.dir/src/flonum-copy.c.o
../as-new: CMakeFiles/as-new.dir/src/flonum-konst.c.o
../as-new: CMakeFiles/as-new.dir/src/flonum-mult.c.o
../as-new: CMakeFiles/as-new.dir/src/frags.c.o
../as-new: CMakeFiles/as-new.dir/src/hash.c.o
../as-new: CMakeFiles/as-new.dir/src/input-file.c.o
../as-new: CMakeFiles/as-new.dir/src/input-scrub.c.o
../as-new: CMakeFiles/as-new.dir/src/listing.c.o
../as-new: CMakeFiles/as-new.dir/src/macro.c.o
../as-new: CMakeFiles/as-new.dir/src/messages.c.o
../as-new: CMakeFiles/as-new.dir/src/output-file.c.o
../as-new: CMakeFiles/as-new.dir/src/read.c.o
../as-new: CMakeFiles/as-new.dir/src/reloc.c.o
../as-new: CMakeFiles/as-new.dir/src/sb.c.o
../as-new: CMakeFiles/as-new.dir/src/subsegs.c.o
../as-new: CMakeFiles/as-new.dir/src/symbols.c.o
../as-new: CMakeFiles/as-new.dir/src/write.c.o
../as-new: CMakeFiles/as-new.dir/config/atof-ieee.c.o
../as-new: CMakeFiles/as-new.dir/config/obj-elf.c.o
../as-new: CMakeFiles/as-new.dir/config/tc-arm.c.o
../as-new: CMakeFiles/as-new.dir/build.make
../as-new: ../../binutils-gdb/bfd/.libs/libbfd.a
../as-new: ../../binutils-gdb/zlib/libz.a
../as-new: ../../binutils-gdb/libiberty/libiberty.a
../as-new: CMakeFiles/as-new.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jjk/project/my_gas/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_29) "Linking C executable ../as-new"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/as-new.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/as-new.dir/build: ../as-new

.PHONY : CMakeFiles/as-new.dir/build

CMakeFiles/as-new.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/as-new.dir/cmake_clean.cmake
.PHONY : CMakeFiles/as-new.dir/clean

CMakeFiles/as-new.dir/depend:
	cd /home/jjk/project/my_gas/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jjk/project/my_gas /home/jjk/project/my_gas /home/jjk/project/my_gas/build /home/jjk/project/my_gas/build /home/jjk/project/my_gas/build/CMakeFiles/as-new.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/as-new.dir/depend

