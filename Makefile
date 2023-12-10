# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/night/Projects/ssched_simulator

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/night/Projects/ssched_simulator

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target test
test:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running tests..."
	/usr/bin/ctest --force-new-ctest-process $(ARGS)
.PHONY : test

# Special rule for the target test
test/fast: test
.PHONY : test/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/night/Projects/ssched_simulator/CMakeFiles /home/night/Projects/ssched_simulator//CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/night/Projects/ssched_simulator/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named ssched_simulator

# Build rule for target.
ssched_simulator: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 ssched_simulator
.PHONY : ssched_simulator

# fast build rule for target.
ssched_simulator/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/ssched_simulator.dir/build.make CMakeFiles/ssched_simulator.dir/build
.PHONY : ssched_simulator/fast

#=============================================================================
# Target rules for targets named injection_model

# Build rule for target.
injection_model: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 injection_model
.PHONY : injection_model

# fast build rule for target.
injection_model/fast:
	$(MAKE) $(MAKESILENT) -f src/traffic_generator/CMakeFiles/injection_model.dir/build.make src/traffic_generator/CMakeFiles/injection_model.dir/build
.PHONY : injection_model/fast

#=============================================================================
# Target rules for targets named traffic_pattern

# Build rule for target.
traffic_pattern: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 traffic_pattern
.PHONY : traffic_pattern

# fast build rule for target.
traffic_pattern/fast:
	$(MAKE) $(MAKESILENT) -f src/traffic_generator/CMakeFiles/traffic_pattern.dir/build.make src/traffic_generator/CMakeFiles/traffic_pattern.dir/build
.PHONY : traffic_pattern/fast

#=============================================================================
# Target rules for targets named scheduler

# Build rule for target.
scheduler: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 scheduler
.PHONY : scheduler

# fast build rule for target.
scheduler/fast:
	$(MAKE) $(MAKESILENT) -f src/scheduler/CMakeFiles/scheduler.dir/build.make src/scheduler/CMakeFiles/scheduler.dir/build
.PHONY : scheduler/fast

#=============================================================================
# Target rules for targets named batch_scheduler

# Build rule for target.
batch_scheduler: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 batch_scheduler
.PHONY : batch_scheduler

# fast build rule for target.
batch_scheduler/fast:
	$(MAKE) $(MAKESILENT) -f src/scheduler/batch_scheduler/CMakeFiles/batch_scheduler.dir/build.make src/scheduler/batch_scheduler/CMakeFiles/batch_scheduler.dir/build
.PHONY : batch_scheduler/fast

#=============================================================================
# Target rules for targets named switch

# Build rule for target.
switch: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 switch
.PHONY : switch

# fast build rule for target.
switch/fast:
	$(MAKE) $(MAKESILENT) -f src/switch/CMakeFiles/switch.dir/build.make src/switch/CMakeFiles/switch.dir/build
.PHONY : switch/fast

#=============================================================================
# Target rules for targets named simulator

# Build rule for target.
simulator: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 simulator
.PHONY : simulator

# fast build rule for target.
simulator/fast:
	$(MAKE) $(MAKESILENT) -f src/simulator/CMakeFiles/simulator.dir/build.make src/simulator/CMakeFiles/simulator.dir/build
.PHONY : simulator/fast

#=============================================================================
# Target rules for targets named sschedsim_test

# Build rule for target.
sschedsim_test: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 sschedsim_test
.PHONY : sschedsim_test

# fast build rule for target.
sschedsim_test/fast:
	$(MAKE) $(MAKESILENT) -f test/CMakeFiles/sschedsim_test.dir/build.make test/CMakeFiles/sschedsim_test.dir/build
.PHONY : sschedsim_test/fast

ssched_simulator.o: ssched_simulator.cc.o
.PHONY : ssched_simulator.o

# target to build an object file
ssched_simulator.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/ssched_simulator.dir/build.make CMakeFiles/ssched_simulator.dir/ssched_simulator.cc.o
.PHONY : ssched_simulator.cc.o

ssched_simulator.i: ssched_simulator.cc.i
.PHONY : ssched_simulator.i

# target to preprocess a source file
ssched_simulator.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/ssched_simulator.dir/build.make CMakeFiles/ssched_simulator.dir/ssched_simulator.cc.i
.PHONY : ssched_simulator.cc.i

ssched_simulator.s: ssched_simulator.cc.s
.PHONY : ssched_simulator.s

# target to generate assembly for a file
ssched_simulator.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/ssched_simulator.dir/build.make CMakeFiles/ssched_simulator.dir/ssched_simulator.cc.s
.PHONY : ssched_simulator.cc.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... test"
	@echo "... batch_scheduler"
	@echo "... injection_model"
	@echo "... scheduler"
	@echo "... simulator"
	@echo "... ssched_simulator"
	@echo "... sschedsim_test"
	@echo "... switch"
	@echo "... traffic_pattern"
	@echo "... ssched_simulator.o"
	@echo "... ssched_simulator.i"
	@echo "... ssched_simulator.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system
