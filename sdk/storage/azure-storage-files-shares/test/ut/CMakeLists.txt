# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-storage-files-shares-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

# Export the test folder for recordings access.
add_compile_definitions(AZURE_TEST_RECORDING_DIR="${CMAKE_CURRENT_LIST_DIR}")

add_executable (
  azure-storage-files-shares-test
    macro_guard.cpp
    share_client_test.cpp
    share_client_test.hpp
    share_directory_client_test.cpp
    share_directory_client_test.hpp
    share_file_attributes_test.cpp
    share_file_client_test.cpp
    share_file_client_test.hpp
    share_sas_test.cpp
    share_service_client_test.cpp
    share_service_client_test.hpp
    # Include shared test source code
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../azure-storage-common/test/ut/test_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../azure-storage-common/test/ut/test_base.hpp
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

create_per_service_target_build(azure-storage-files-shares-test storage)
create_map_file(azure-storage-files-shares-test azure-storage-files-shares-test.map)

# Include shared test headers
target_include_directories(azure-storage-files-shares-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../azure-storage-common)

target_link_libraries(azure-storage-files-shares-test PRIVATE azure-storage-files-shares azure-storage-blobs azure-identity azure-core-test-fw gtest gtest_main gmock)

# gtest_discover_tests will scan the test from azure-storage-files-shares-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-storage-files-shares-test
     TEST_PREFIX azure-storage-files-shares.
     NO_PRETTY_TYPES
     NO_PRETTY_VALUES)
