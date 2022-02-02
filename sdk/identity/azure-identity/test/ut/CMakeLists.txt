# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

set(azure-identity-test)

project (azure-identity-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

# Export the test folder for recordings access.
add_compile_definitions(AZURE_TEST_RECORDING_DIR="${CMAKE_CURRENT_LIST_DIR}")

add_executable (
  azure-identity-test
    client_secret_credential_test.cpp
    credential_test_helper.cpp
    credential_test_helper.hpp
    environment_credential_test.cpp
    macro_guard_test.cpp
    managed_identity_credential_test.cpp
    simplified_header_test.cpp
    token_credential_impl_test.cpp
    token_credential_test.cpp
    )

if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
  # cspell:words VCRUNTIME
  set(AZ_UWP_VCRUNTIME_PATH "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Redist/MSVC/14.29.30133/onecore/debug_nonredist/x86/Microsoft.VC142.DebugCRT/VCRUNTIME140D.dll")
  set(AZ_UWP_BUILD_CONFIG "Debug")
  if (EXISTS "${VCRUNTIME_PATH}")
    get_filename_component(AZ_UWP_VCRUNTIME_FILENAME "${VCRUNTIME_PATH}" NAME_WE)
    add_custom_command(
        TARGET azure-identity-test
          POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E "copy_if_different \"${AZ_UWP_VCRUNTIME_PATH}\" ${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}/"
            COMMAND ${CMAKE_COMMAND} -E "rename \"${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}/${AZ_UWP_VCRUNTIME_FILENAME}.dll ${CMAKE_CURRENT_BINARY_DIR}/${AZ_UWP_BUILD_CONFIG}/${AZ_UWP_VCRUNTIME_FILENAME}_APP.dll"
    )
    unset(AZ_UWP_VCRUNTIME_FILENAME)
  endif()
endif()
  
create_per_service_target_build(azure-identity-test identity)

create_map_file(azure-identity-test azure-identity-test.map)

if (MSVC)
  # Disable warnings:
  # - C26495: Variable
  #             - 'testing::internal::Mutex::critical_section_'
  #             - 'testing::internal::Mutex::critical_section_init_phase_'
  #             - 'testing::internal::Mutex::owner_thread_id_'
  #             - 'testing::internal::Mutex::type_'
  #           is uninitialized. Always initialize member variables (type.6).
  # - C26812: The enum type
  #             - 'testing::internal::Mutex::StaticConstructorSelector'
  #             - 'testing::TestPartResult::Type'
  #           is unscoped. Prefer 'enum class' over 'enum' (Enum.3)
  target_compile_options(azure-identity-test PUBLIC /wd26495 /wd26812 /wd6326)
endif()

# Adding private headers from identity to the tests so we can test the private APIs with no relative paths include.
target_include_directories (azure-identity-test PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>)

target_link_libraries(azure-identity-test PRIVATE azure-identity azure-core-test-fw gtest gtest_main gmock)

# gtest_discover_tests will scan the test from azure-identity-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-identity-test
     TEST_PREFIX azure-identity.
     NO_PRETTY_TYPES
     NO_PRETTY_VALUES)
