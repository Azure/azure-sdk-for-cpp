# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

# CodeCoverage will validate requirements
include(CodeCoverage)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
# excluding json from coverage report
create_code_coverage(storage azure-storage-common azure-storage-test "inc/azure/storage/common/json*")

target_sources(
    azure-storage-test
    PRIVATE
    bearer_token_test.cpp
    crypt_functions_test.cpp
    metadata_test.cpp
    storage_credential_test.cpp
    test_base.cpp
)

target_link_libraries(azure-storage-test PRIVATE Azure::azure-identity)

target_include_directories(azure-storage-test PRIVATE .)
if (MSVC)
    target_compile_options(azure-storage-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()
