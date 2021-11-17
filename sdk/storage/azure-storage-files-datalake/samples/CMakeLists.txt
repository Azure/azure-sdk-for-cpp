# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

add_executable(datalake_getting_started datalake_getting_started.cpp)
target_link_libraries(datalake_getting_started PRIVATE azure-storage-files-datalake)
