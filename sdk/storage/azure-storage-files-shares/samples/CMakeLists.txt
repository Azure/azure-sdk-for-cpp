# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

add_executable(file-share-getting-started file_share_getting_started.cpp)
target_link_libraries(file-share-getting-started PRIVATE azure-storage-files-shares)
