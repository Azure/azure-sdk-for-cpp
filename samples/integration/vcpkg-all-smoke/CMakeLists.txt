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
project(smoketest-vcpkg LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include(AzureVcpkg)
az_vcpkg_integrate()

find_package(azure-storage-blobs-cpp REQUIRED)
find_package(azure-storage-files-datalake-cpp REQUIRED)
find_package(azure-storage-files-shares-cpp REQUIRED)
find_package(azure-storage-queues-cpp REQUIRED)
find_package(azure-security-keyvault-keys-cpp REQUIRED)
find_package(azure-security-keyvault-certificates-cpp REQUIRED)
find_package(azure-security-keyvault-secrets-cpp REQUIRED)
find_package(azure-identity-cpp REQUIRED)
find_package(azure-core-cpp REQUIRED)

add_executable (
  smoketest-vcpkg
    src/main
)

# Link to Azure SDK
target_link_libraries(smoketest-vcpkg
     PRIVATE
     Azure::azure-storage-blobs
     Azure::azure-storage-files-datalake
     Azure::azure-storage-files-shares
     Azure::azure-storage-queues
     Azure::azure-security-keyvault-keys
     Azure::azure-security-keyvault-certificates
     Azure::azure-security-keyvault-secrets
     Azure::azure-identity
     Azure::azure-core
     get-env-helper
)

# Add the sample to be run on CI. 
# CI pipeline reads the {service}-samples.txt and runs the binaries listed there.
file(APPEND ${CMAKE_BINARY_DIR}/keyvault-samples.txt "${CMAKE_CURRENT_BINARY_DIR}/smoketest-vcpkg\n")
