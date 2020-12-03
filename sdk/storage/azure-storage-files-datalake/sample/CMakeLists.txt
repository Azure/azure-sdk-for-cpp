# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

target_sources(
    azure-storage-sample
    PRIVATE
    datalake_getting_started.cpp
)
target_link_libraries(azure-storage-sample PRIVATE azure-storage-files-datalake)
