# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# vcpkg integration.
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "")
endif()
if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
  set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

# Project set up
cmake_minimum_required(VERSION 3.13)
project(vcpkg-all-smoke LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  vcpkg-all-smoke
    src/main
)

# Link to Azure SDK
target_link_libraries(vcpkg-all-smoke
     PRIVATE
     Azure::azure-core
     Azure::azure-identity
     Azure::azure-storage-blobs
     Azure::azure-storage-files-datalake
     Azure::azure-storage-files-shares
     Azure::azure-storage-queues
     Azure::azure-security-keyvault-keys
     Azure::azure-security-keyvault-certificates
     Azure::azure-security-keyvault-secrets
     get-env-helper
)

create_per_service_target_build_for_sample(storage transactional-checksum)
# Add the sample to be run on CI. 
# CI pipeline reads the {service}-samples.txt and runs the binaries listed there.
# file(APPEND ${CMAKE_BINARY_DIR}/keyvault-samples.txt "${CMAKE_CURRENT_BINARY_DIR}/vcpkg-all-smoke\n")
