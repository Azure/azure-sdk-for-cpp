# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-template-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(AZURE_TEST_DATA_PATH="${CMAKE_BINARY_DIR}")

include(GoogleTest)

add_executable (
     azure-template-test
     ut/macro_guard.cpp
     ut/template_test.cpp
     )

# cspell:words VCRUNTIME
set(AZ_UWP_VCRUNTIME_PATH "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Redist/MSVC/14.29.30133/onecore/debug_nonredist/x86/Microsoft.VC142.DebugCRT/VCRUNTIME140D.dll")
set(AZ_UWP_BUILD_CONFIG "Debug")
if (EXISTS "${VCRUNTIME_PATH}")
  get_filename_component(AZ_UWP_VCRUNTIME_FILENAME "${VCRUNTIME_PATH}" NAME_WE)
  add_custom_command(
    OUTPUT vcruntime_app_dll_copied.txt
    COMMAND ${CMAKE_COMMAND} -E echo "${AZ_UWP_VCRUNTIME_PATH} +> ${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}"
    COMMAND ${CMAKE_COMMAND} -E echo "${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}/${AZ_UWP_VCRUNTIME_FILENAME}.dll => ${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}/${AZ_UWP_VCRUNTIME_FILENAME}_APP.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${AZ_UWP_VCRUNTIME_PATH}" ${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E rename "${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}/${AZ_UWP_VCRUNTIME_FILENAME}.dll" ${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}/${AZ_UWP_VCRUNTIME_FILENAME}_APP.dll
  )
  unset(AZ_UWP_VCRUNTIME_FILENAME)
endif()

create_per_service_target_build(azure-template-test template)
create_map_file(azure-template-test azure-template-test.map)

target_link_libraries(azure-template-test PRIVATE Azure::azure-template gtest_main)

if (MSVC)
    target_compile_options(azure-template-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

gtest_discover_tests(azure-template-test TEST_PREFIX azure-template.)
