# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (azure-data-tables-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

# Export the test folder for recordings access.
add_compile_definitions(AZURE_TEST_RECORDING_DIR="${CMAKE_CURRENT_LIST_DIR}")
include(TestProxyPrep)
SetUpTestProxy("sdk/tables")

add_executable (
  azure-data-tables-test
    enum_operators_test.hpp
    enum_operators_test.cpp
    macro_guard.cpp
    sas_test.cpp
    serializers_test.hpp
    serializers_test.cpp
    shared_key_lite_policy_test.cpp
    table_client_test.cpp
    table_client_test.hpp
    transactions_test.hpp
    transactions_test.cpp
    test_base.hpp
    test_base.cpp
    )

target_compile_definitions(azure-data-tables-test PRIVATE _azure_BUILDING_TESTS)

create_per_service_target_build(tables azure-data-tables-test)
create_map_file(azure-data-tables-test azure-data-tables-test.map)

target_link_libraries(azure-data-tables-test PRIVATE azure-data-tables azure-identity azure-core-test-fw gtest gtest_main gmock)

# gtest_discover_tests will scan the test from azure-data-tables-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-data-tables-test
     TEST_PREFIX azure-data-tables.
     NO_PRETTY_TYPES
     NO_PRETTY_VALUES
     DISCOVERY_TIMEOUT 600)
