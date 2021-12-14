# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-storage LANGUAGES CXX)
option(BUILD_STORAGE_SAMPLES "Build storage sample codes" ON)

if(BUILD_TESTING)

  if (NOT AZ_ALL_LIBRARIES)
    include(AddGoogleTest)
    enable_testing ()
  endif()

  add_executable(azure-storage-test)
  add_gtest(azure-storage-test)
endif()

add_subdirectory(azure-storage-common)
add_subdirectory(azure-storage-blobs)
add_subdirectory(azure-storage-files-datalake)
add_subdirectory(azure-storage-files-shares)
add_subdirectory(azure-storage-queues)
