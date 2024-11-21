# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

#cspell: words tpmattestation
cmake_minimum_required (VERSION 3.13)

project (azure-security-attestation-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CreateMapFile)


add_compile_definitions(AZURE_TEST_DATA_PATH="${CMAKE_BINARY_DIR}")

# Export the test folder for recordings access.
add_compile_definitions(AZURE_TEST_RECORDING_DIR="${CMAKE_CURRENT_LIST_DIR}")

include(TestProxyPrep)
SetUpTestProxy("sdk/attestation")

include(GoogleTest)

add_executable (
     azure-security-attestation-test
     attestation_collateral.cpp
     attestation_metadata.cpp
     attestation_test.cpp
     crypto_test.cpp
     crypto_test_collateral.cpp
     crypto_test_collateral.hpp
     macro_guard.cpp
     policycertmgmt_test.cpp
     policygetset_test.cpp
     token_test.cpp
     tpmattestation_test.cpp
)

target_compile_definitions(azure-security-attestation-test PRIVATE _azure_BUILDING_TESTS)

create_per_service_target_build(attestation azure-security-attestation-test)

create_map_file(azure-security-attestation-test azure-security-attestation-test.map)

target_link_libraries(azure-security-attestation-test PRIVATE Azure::azure-security-attestation azure-identity azure-core-test-fw gtest_main)


gtest_discover_tests(azure-security-attestation-test 
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DISCOVERY_TIMEOUT 600 
    TEST_PREFIX azure-security-attestation.)
