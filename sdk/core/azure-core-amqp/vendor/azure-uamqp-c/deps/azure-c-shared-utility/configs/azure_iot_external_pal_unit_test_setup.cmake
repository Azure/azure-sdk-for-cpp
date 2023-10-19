#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

# quiet the CMake warning about unused command line args "use_wsio". This is from the c-utility CMake command.
set(ignore_me ${use_wsio})

# Set up to make an external repo's unit tests
include("${CMAKE_CURRENT_LIST_DIR}/azure_iot_build_rules.cmake")
include("${SHARED_UTIL_FOLDER}/dependencies-test.cmake")

add_subdirectory(${SHARED_UTIL_FOLDER}/testtools/ctest)
add_subdirectory(${SHARED_UTIL_FOLDER}/testtools/testrunner)
add_subdirectory(${SHARED_UTIL_FOLDER}/testtools/umock-c)

#Setup the platform files in order to include the logging file definitions
include("${SHARED_UTIL_FOLDER}/configs/azure_c_shared_utilityFunctions.cmake")
set_platform_files(${SHARED_UTIL_FOLDER})
