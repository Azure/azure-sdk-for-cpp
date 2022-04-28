# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-storage LANGUAGES CXX)

add_subdirectory(azure-storage-common)
add_subdirectory(azure-storage-blobs)
add_subdirectory(azure-storage-files-datalake)
add_subdirectory(azure-storage-files-shares)
add_subdirectory(azure-storage-queues)
add_subdirectory(azure-storage-datamovement)
