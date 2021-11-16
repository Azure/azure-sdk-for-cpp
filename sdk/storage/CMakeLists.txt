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

if(BUILD_STORAGE_SAMPLES)
  add_executable(azure-storage-sample)
  # Add the sample to be run on CI.
  # CI pipeline reads the {service}-samples.txt and runs the binaries listed there.
  file(APPEND ${CMAKE_BINARY_DIR}/storage-samples.txt "${CMAKE_CURRENT_BINARY_DIR}/azure-storage-sample All\n")
endif()

add_subdirectory(azure-storage-common)
add_subdirectory(azure-storage-blobs)
add_subdirectory(azure-storage-files-datalake)
add_subdirectory(azure-storage-files-shares)
add_subdirectory(azure-storage-queues)
