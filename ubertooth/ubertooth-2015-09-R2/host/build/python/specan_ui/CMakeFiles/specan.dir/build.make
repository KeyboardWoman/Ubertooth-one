# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

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
CMAKE_SOURCE_DIR = /home/opensourcesdr/SDR/ubertooth/ubertooth-2015-09-R2/host

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/opensourcesdr/SDR/ubertooth/ubertooth-2015-09-R2/host/build

# Utility rule file for specan.

# Include the progress variables for this target.
include python/specan_ui/CMakeFiles/specan.dir/progress.make

python/specan_ui/CMakeFiles/specan: python/specan_ui/build/timestamp


python/specan_ui/build/timestamp: ../python/specan_ui/specan/__init__.py
python/specan_ui/build/timestamp: ../python/specan_ui/specan/Ubertooth.py
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/opensourcesdr/SDR/ubertooth/ubertooth-2015-09-R2/host/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating build/timestamp"
	cd /home/opensourcesdr/SDR/ubertooth/ubertooth-2015-09-R2/host/build/python/specan_ui && /usr/bin/python setup.py build
	cd /home/opensourcesdr/SDR/ubertooth/ubertooth-2015-09-R2/host/build/python/specan_ui && /usr/bin/cmake -E touch /home/opensourcesdr/SDR/ubertooth/ubertooth-2015-09-R2/host/build/python/specan_ui/build/timestamp

specan: python/specan_ui/CMakeFiles/specan
specan: python/specan_ui/build/timestamp
specan: python/specan_ui/CMakeFiles/specan.dir/build.make

.PHONY : specan

# Rule to build all files generated by this target.
python/specan_ui/CMakeFiles/specan.dir/build: specan

.PHONY : python/specan_ui/CMakeFiles/specan.dir/build

python/specan_ui/CMakeFiles/specan.dir/clean:
	cd /home/opensourcesdr/SDR/ubertooth/ubertooth-2015-09-R2/host/build/python/specan_ui && $(CMAKE_COMMAND) -P CMakeFiles/specan.dir/cmake_clean.cmake
.PHONY : python/specan_ui/CMakeFiles/specan.dir/clean

python/specan_ui/CMakeFiles/specan.dir/depend:
	cd /home/opensourcesdr/SDR/ubertooth/ubertooth-2015-09-R2/host/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/opensourcesdr/SDR/ubertooth/ubertooth-2015-09-R2/host /home/opensourcesdr/SDR/ubertooth/ubertooth-2015-09-R2/host/python/specan_ui /home/opensourcesdr/SDR/ubertooth/ubertooth-2015-09-R2/host/build /home/opensourcesdr/SDR/ubertooth/ubertooth-2015-09-R2/host/build/python/specan_ui /home/opensourcesdr/SDR/ubertooth/ubertooth-2015-09-R2/host/build/python/specan_ui/CMakeFiles/specan.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : python/specan_ui/CMakeFiles/specan.dir/depend

