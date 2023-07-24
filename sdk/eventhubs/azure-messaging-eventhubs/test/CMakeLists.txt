# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.13)

# Include sub-projects.
add_subdirectory ("ut")
# stress tests are categorized as normal tests.
add_subdirectory ("eventhubs-stress-test")
if (BUILD_PERFORMANCE_TESTS) 
add_subdirectory ("perf")
endif()
