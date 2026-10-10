# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (azure-identity-e2e-test LANGUAGES CXX)

add_executable (
  azure-identity-e2e-test
    azure_identity_e2e_test.cpp
)
target_compile_features(azure-identity-e2e-test PRIVATE cxx_std_14)
set_target_properties(azure-identity-e2e-test PROPERTIES CXX_STANDARD_REQUIRED ON)
create_per_service_target_build(identity azure-identity-e2e-test)

target_compile_definitions(azure-identity-e2e-test PRIVATE _azure_BUILDING_TESTS)

create_map_file(azure-identity-e2e-test azure-identity-e2e-test.map)
target_link_libraries(azure-identity-e2e-test PRIVATE azure-identity)
