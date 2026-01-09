# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (azure-data-appconfiguration-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CreateMapFile)

add_compile_definitions(AZURE_TEST_DATA_PATH="${CMAKE_BINARY_DIR}")

include(GoogleTest)

add_executable (
     azure-data-appconfiguration-test
     configuration_test.cpp
     macro_guard.cpp
     )

target_compile_definitions(azure-data-appconfiguration-test PRIVATE _azure_BUILDING_TESTS)

create_per_service_target_build(appconfiguration azure-data-appconfiguration-test)
create_map_file(azure-data-appconfiguration-test azure-data-appconfiguration-test.map)

target_link_libraries(azure-data-appconfiguration-test PRIVATE Azure::azure-data-appconfiguration azure-identity azure-core-test-fw gtest gtest_main gmock)

if (MSVC)
    target_compile_options(azure-data-appconfiguration-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

# Automated runtime dependency copying
copy_shared_lib_binaries(azure-data-appconfiguration-test Azure::azure-data-appconfiguration azure-core azure-identity azure-core-test-fw GTest::gtest GTest::gmock)

gtest_discover_tests(azure-data-appconfiguration-test 
     TEST_PREFIX azure-data-appconfiguration.
     DISCOVERY_TIMEOUT 600)
