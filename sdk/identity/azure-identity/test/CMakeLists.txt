# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

set(azure-identity-test)

project (azure-identity-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

add_executable (
  azure-identity-test
    main.cpp
    simplified_header.cpp
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
  target_compile_options(azure-identity-test PUBLIC /wd26495 /wd26812)
endif()

# Adding private headers from identity to the tests so we can test the private APIs with no relative paths include.
target_include_directories (azure-identity-test PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>)

target_link_libraries(azure-identity-test PRIVATE azure-identity gtest gmock)

# gtest_discover_tests will scan the test from azure-identity-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-identity-test
     TEST_PREFIX azure-identity.
     NO_PRETTY_TYPES
     NO_PRETTY_VALUES)

if (BUILD_PERFORMANCE_TESTS)
  add_subdirectory(performance)
endif()
