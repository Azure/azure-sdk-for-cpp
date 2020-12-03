# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

# CodeCoverage will validate requirements
include(CodeCoverage)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(storage azure-storage-files-datalake azure-storage-test)

target_sources(
    azure-storage-test
    PRIVATE
    datalake_directory_client_test.cpp
    datalake_file_client_test.cpp
    datalake_file_system_client_test.cpp
    datalake_path_client_test.cpp
    datalake_sas_test.cpp
    datalake_service_client_test.cpp
)

target_link_libraries(azure-storage-test PRIVATE azure-storage-files-datalake)
