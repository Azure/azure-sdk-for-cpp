# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.15)

if (BUILD_CURL_TRANSPORT)

add_executable (
     azure-storage-sample
     # blob_getting_started.cpp
     datalake_getting_started.cpp
)

target_link_libraries(azure-storage-sample PRIVATE azure-storage)

endif()
