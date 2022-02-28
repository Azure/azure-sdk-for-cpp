# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (attestation-basic-operations LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  attestation-attestation
  attestation.cpp
)

create_per_service_target_build(attestation-attestation attestation)

target_link_libraries(attestation-attestation PRIVATE azure-security-attestation azure-identity get-env-helper)

# Add the sample to be run on CI. 
# CI pipeline reads the {service}-samples.txt and runs the binaries listed there.
file(APPEND ${CMAKE_BINARY_DIR}/attestation-samples.txt "${CMAKE_CURRENT_BINARY_DIR}/attestation-attestation\n")
