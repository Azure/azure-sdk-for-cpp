# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.15)

add_executable (
    azure-storage-sample
    main.cpp
    samples_common.hpp
    blob_getting_started.cpp
    #datalake_getting_started.cpp
)

target_link_libraries(azure-storage-sample azure::storage::blob)
