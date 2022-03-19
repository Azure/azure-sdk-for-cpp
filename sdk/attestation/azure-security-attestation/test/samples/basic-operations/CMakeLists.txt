# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (attestation-basic-operations LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  attestation-basic-operations
  basic-operations.cpp
)

create_per_service_target_build(attestation-basic-operations attestation)

target_link_libraries(attestation-basic-operations PRIVATE azure-security-attestation azure-identity get-env-helper)

# Add the sample to be run on CI. 
# CI pipeline reads the {service}-samples.txt and runs the binaries listed there.
add_sample_for_ci_run(attestation attestation-basic-operations)
