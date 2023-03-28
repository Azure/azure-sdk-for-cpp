# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT


cmake_minimum_required (VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(azure-core-amqp-tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(AzureBuildTargetForCI)
include(GoogleTest)


# Unit Tests
add_executable(azure-core-amqp-tests
    "amqp_value_tests.cpp"
    "amqp_header_tests.cpp"
    "amqp_properties_tests.cpp"
    "amqp_message_tests.cpp"
    "connection_tests.cpp"
    "transport_tests.cpp" 
    "session_tests.cpp"
    "link_tests.cpp" 
    "message_sender_receiver.cpp"
    "connection_string_tests.cpp"
 claim_based_security_tests.cpp "message_source_target.cpp")
add_dependencies(azure-core-amqp-tests azure-core-amqp)

target_include_directories(azure-core-amqp-tests PRIVATE ${azure-core-amqp_SOURCE_DIR})

# Link test executable against gtest & gtest_main
target_link_libraries(azure-core-amqp-tests PRIVATE GTest::gtest_main azure-core-amqp)
gtest_discover_tests( azure-core-amqp-tests )

create_per_service_target_build(core azure-core-amqp-tests)
create_map_file(azure-core-amqp-tests azure-core-amqp-tests.map)

# gtest_discover_tests will scan the test from azure-core-tracing-opentelemetry-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-core-amqp-tests
     TEST_PREFIX azure-core-amqp.
     DISCOVERY_TIMEOUT 600)
