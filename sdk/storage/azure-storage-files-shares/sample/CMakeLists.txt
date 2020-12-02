# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

target_sources(
    azure-storage-sample
    PRIVATE
    file_share_getting_started.cpp
)

target_link_libraries(azure-storage-sample PRIVATE Azure::azure-storage-files-shares)
