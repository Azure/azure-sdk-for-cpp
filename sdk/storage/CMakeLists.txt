# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (azure-storage LANGUAGES CXX)

add_subdirectory(azure-storage-common)
add_subdirectory(azure-storage-blobs)
add_subdirectory(azure-storage-files-datalake)
add_subdirectory(azure-storage-files-shares)
add_subdirectory(azure-storage-queues)

