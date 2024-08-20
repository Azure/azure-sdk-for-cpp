# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.13)

# Include sub-projects.

# General AMQP Unit tests.
add_subdirectory ("ut")

if (USE_UAMQP)
# Unit tests which require using the uamqp library.
add_subdirectory ("ut_uamqp")
endif()
