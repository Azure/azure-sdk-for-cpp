# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

set(azure-core-tracing-opentelemetry-test)

add_compile_definitions(AZURE_TEST_DATA_PATH="${CMAKE_BINARY_DIR}")

add_compile_definitions(AZURE_TEST_RECORDING_DIR="${CMAKE_CURRENT_LIST_DIR}")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  add_compile_options(-Wno-error=deprecated-declarations)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
# Disable deprecation warnings.
  add_compile_options(/wd4996)
endif()


project (azure-core-tracing-opentelemetry-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(AzureBuildTargetForCI)
include(GoogleTest)

add_executable (
  azure-core-tracing-opentelemetry-test
    azure_core_otel_test.cpp
    azure_core_test.cpp service_support_test.cpp)

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
  # - C6326: Google comparisons 
  target_compile_options(azure-core-test PUBLIC /wd26495 /wd26812 /wd6326 /wd28204 /wd28020 /wd6330 /wd4389)
endif()

# Adding private headers from CORE to the tests so we can test the private APIs with no relative paths include.
target_include_directories (azure-core-tracing-opentelemetry-test PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>)

target_link_libraries(azure-core-tracing-opentelemetry-test PRIVATE azure-core-tracing-opentelemetry 
    azure-core
    azure-identity
    opentelemetry-cpp::ostream_span_exporter
    opentelemetry-cpp::in_memory_span_exporter
    opentelemetry-cpp::sdk
    azure-core-test-fw 
    gtest_main)

create_per_service_target_build(core azure-core-tracing-opentelemetry-test)
create_map_file(azure-core-tracing-opentelemetry-test azure-core-tracing-opentelemetry-test.map)

# gtest_discover_tests will scan the test from azure-core-tracing-opentelemetry-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-core-tracing-opentelemetry-test
     TEST_PREFIX azure-core-tracing-opentelemetry.
     NO_PRETTY_TYPES
     NO_PRETTY_VALUES
     DISCOVERY_TIMEOUT 600)
