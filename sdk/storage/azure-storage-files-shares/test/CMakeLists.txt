# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

# CodeCoverage will validate requirements
include(CodeCoverage)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(storage azure-storage-files-shares azure-storage-test)

target_sources(
    azure-storage-test
    PRIVATE
    share_client_test.cpp
    share_client_test.hpp
    share_directory_client_test.cpp
    share_directory_client_test.hpp
    share_file_client_test.cpp
    share_file_client_test.hpp
    share_sas_test.cpp
    share_service_client_test.cpp
    share_service_client_test.hpp
)

target_link_libraries(azure-storage-test PRIVATE azure-storage-files-shares)
