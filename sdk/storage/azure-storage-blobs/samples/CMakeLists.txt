# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

add_executable(blob_getting_started blob_getting_started.cpp)
target_link_libraries(blob_getting_started PRIVATE azure-storage-blobs)

add_executable(blob_list_operation blob_list_operation.cpp)
target_link_libraries(blob_list_operation PRIVATE azure-storage-blobs)

add_executable(blob_sas blob_sas.cpp)
target_link_libraries(blob_sas PRIVATE azure-storage-blobs)

add_executable(transactional_checksum transactional_checksum.cpp)
target_link_libraries(transactional_checksum PRIVATE azure-storage-blobs)
