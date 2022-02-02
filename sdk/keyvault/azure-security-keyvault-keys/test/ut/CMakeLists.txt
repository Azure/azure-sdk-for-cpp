# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-security-keyvault-keys-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

# Export the test folder for recordings access.
add_compile_definitions(AZURE_TEST_RECORDING_DIR="${CMAKE_CURRENT_LIST_DIR}")

################## Unit Tests ##########################
add_executable (
  azure-security-keyvault-keys-test
    key_client_backup_test_live.cpp
    key_client_base_test.hpp
    key_client_base_test.hpp
    key_client_create_test_live.cpp
    key_client_delete_test_live.cpp
    key_client_get_test_live.cpp
    key_client_import_test_live.cpp
    key_client_test.cpp
    key_client_update_test_live.cpp
    key_cryptographic_client_test_live.cpp
    macro_guard.cpp
    mocked_client_test.cpp
    mocked_transport_adapter_test.hpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
  # cspell:words VCRUNTIME
  set(AZ_UWP_VCRUNTIME_PATH "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Redist/MSVC/14.29.30133/onecore/debug_nonredist/x86/Microsoft.VC142.DebugCRT/VCRUNTIME140D.dll")
  set(AZ_UWP_BUILD_CONFIG "Debug")
  if (EXISTS "${VCRUNTIME_PATH}")
    get_filename_component(AZ_UWP_VCRUNTIME_FILENAME "${VCRUNTIME_PATH}" NAME_WE)
    add_custom_command(
        TARGET azure-core-global-context-test
          POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E "copy_if_different \"${AZ_UWP_VCRUNTIME_PATH}\" ${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}/"
            COMMAND ${CMAKE_COMMAND} -E "rename \"${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}/${AZ_UWP_VCRUNTIME_FILENAME}.dll ${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}/${AZ_UWP_VCRUNTIME_FILENAME}_APP.dll"
    )
    unset(AZ_UWP_VCRUNTIME_FILENAME)
  endif()
endif()

create_per_service_target_build(azure-security-keyvault-keys-test keyvault)
create_map_file(azure-security-keyvault-keys-test azure-security-keyvault-keys-test.map)

if (MSVC)
  target_compile_options(azure-security-keyvault-keys-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(azure-security-keyvault-keys-test PRIVATE azure-security-keyvault-keys azure-identity azure-core-test-fw gtest gtest_main gmock)

# Adding private headers so we can test the private APIs with no relative paths include.
target_include_directories (azure-security-keyvault-keys-test PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>)

# gtest_add_tests will scan the test from azure-core-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-security-keyvault-keys-test
  TEST_PREFIX azure-security-keyvault-keys.
  NO_PRETTY_TYPES
  NO_PRETTY_VALUES
)
