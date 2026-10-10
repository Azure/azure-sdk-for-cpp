# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

set(azure-core-test-fault-injector)

project (azure-core-test-fault-injector LANGUAGES CXX)

add_executable (
  azure-core-test-fault-injector
  fault_injector_test.cpp
)
target_compile_features(azure-core-test-fault-injector PRIVATE cxx_std_14)
set_target_properties(azure-core-test-fault-injector PROPERTIES CXX_STANDARD_REQUIRED ON)

target_link_libraries(azure-core-test-fault-injector PRIVATE azure-core)

target_compile_definitions(azure-core-test-fault-injector PRIVATE _azure_BUILDING_TESTS)

create_map_file(azure-core-test-fault-injector azure-core-test-fault-injector.map)
