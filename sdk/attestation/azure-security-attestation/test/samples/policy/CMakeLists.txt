# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (attestation-policy LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  attestation-policy
  policy.cpp
)

create_per_service_target_build(attestation-policy attestation true)

target_link_libraries(attestation-policy PRIVATE azure-security-attestation azure-identity azure-core get-env-helper)
