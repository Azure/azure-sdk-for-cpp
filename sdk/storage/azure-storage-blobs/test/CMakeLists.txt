# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

# CodeCoverage will validate requirements
include(CodeCoverage)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(storage azure-storage-blobs azure-storage-test)

target_sources(
    azure-storage-test
    PRIVATE
    append_blob_client_test.cpp
    blob_batch_client_test.cpp
    blob_container_client_test.cpp
    blob_sas_test.cpp
    blob_service_client_test.cpp
    block_blob_client_test.cpp
    page_blob_client_test.cpp
    storage_retry_policy_test.cpp
)

target_link_libraries(azure-storage-test PRIVATE Azure::azure-storage-blobs)
