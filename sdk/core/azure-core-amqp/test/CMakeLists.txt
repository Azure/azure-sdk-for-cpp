# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.13)

# Include sub-projects.
add_subdirectory ("ut")
add_subdirectory ("ut_uamqp")
