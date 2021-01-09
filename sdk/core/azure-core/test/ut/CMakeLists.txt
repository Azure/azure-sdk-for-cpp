# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

set(azure-core-test)

# Create test data for FileUpload test (100K) by writing 1K * 100 times
set(RANGE 0)
set(1K "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")
file(WRITE ${CMAKE_BINARY_DIR}/fileData "")
while(RANGE LESS 100)
     file(APPEND ${CMAKE_BINARY_DIR}/fileData "${1K}")
     MATH(EXPR RANGE "${RANGE}+1")
endwhile()
add_compile_definitions(AZURE_TEST_DATA_PATH="${CMAKE_BINARY_DIR}")

project (azure-core-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(BUILD_TRANSPORT_CURL)
  SET(CURL_OPTIONS_TESTS curl_options.cpp)
  SET(CURL_SESSION_TESTS curl_session_test.cpp)
  SET(CURL_CONNECTION_POOL_TESTS curl_connection_pool.cpp)
endif()

include(GoogleTest)

add_executable (
  azure-core-test
    base64.cpp
    context.cpp
    ${CURL_CONNECTION_POOL_TESTS}
    ${CURL_OPTIONS_TESTS}
    ${CURL_SESSION_TESTS}
    datetime.cpp
    http.cpp
    json.cpp
    logging.cpp
    main.cpp
    nullable.cpp
    operation.cpp
    operation_status.cpp
    pipeline.cpp
    policy.cpp
    simplified_header.cpp
    string.cpp
    telemetry_policy.cpp
    transport_adapter_base.cpp
    transport_adapter_implementation.cpp
    url.cpp
    uuid.cpp
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
  target_compile_options(azure-core-test PUBLIC /wd26495 /wd26812)
endif()

# Adding private headers from CORE to the tests so we can test the private APIs with no relative paths include.
target_include_directories (azure-core-test PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>)

target_link_libraries(azure-core-test PRIVATE azure-core gtest gmock)

# gtest_discover_tests will scan the test from azure-core-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-core-test
     TEST_PREFIX azure-core.
     NO_PRETTY_TYPES
     NO_PRETTY_VALUES)
