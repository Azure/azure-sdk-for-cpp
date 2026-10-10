# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (keyvault-secrets-test-app LANGUAGES CXX)

add_executable (
  keyvault-secrets-test-app
    test_app.cpp
    macro_guard.cpp
 )
target_compile_features(keyvault-secrets-test-app PRIVATE cxx_std_14)
set_target_properties(keyvault-secrets-test-app PROPERTIES CXX_STANDARD_REQUIRED ON)

target_compile_definitions(keyvault-secrets-test-app PRIVATE _azure_BUILDING_TESTS)

create_per_service_target_build(keyvault keyvault-secrets-test-app)

create_map_file(keyvault-secrets-test-app keyvault-secrets-test-app.map)

if (MSVC)
  target_compile_options(keyvault-secrets-test-app PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(keyvault-secrets-test-app PRIVATE azure-security-keyvault-secrets azure-identity)
