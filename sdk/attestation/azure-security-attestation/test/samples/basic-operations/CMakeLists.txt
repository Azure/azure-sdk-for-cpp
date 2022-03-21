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

create_per_service_target_build_for_sample(attestation attestation-basic-operations)

target_link_libraries(attestation-basic-operations PRIVATE azure-security-attestation azure-identity get-env-helper)
