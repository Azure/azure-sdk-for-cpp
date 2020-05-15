# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.12)

add_executable (
     azure-storage-test
     block_blob_client_test.cpp 
     main.cpp 
)

target_link_libraries(azure-storage-test PRIVATE azure-storage)

add_gtest(azure-storage-test)

