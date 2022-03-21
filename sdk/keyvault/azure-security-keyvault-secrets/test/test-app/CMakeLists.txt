# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (keyvault-secrets-test-app LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  keyvault-secrets-test-app
    test_app.cpp
    macro_guard.cpp
 )

create_per_service_target_build(keyvault keyvault-secrets-test-app)

create_map_file(keyvault-secrets-test-app keyvault-secrets-test-app.map)

if (MSVC)
  target_compile_options(keyvault-secrets-test-app PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(keyvault-secrets-test-app PRIVATE azure-security-keyvault-secrets azure-identity)
