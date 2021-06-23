# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-identity-livetest LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

add_executable (
  azure-identity-livetest
    azure_identity_live_test.cpp
    client_secret_credential_test.cpp
    environment_credential_test.cpp
    getenv.hpp
)

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
  target_compile_options(azure-identity-livetest PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(azure-identity-livetest PRIVATE azure-identity gtest gmock)

# gtest_discover_tests will scan the test from azure-core-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-identity-livetest
  TEST_PREFIX azure-identity-livetest.
  NO_PRETTY_TYPES
  NO_PRETTY_VALUES
)
