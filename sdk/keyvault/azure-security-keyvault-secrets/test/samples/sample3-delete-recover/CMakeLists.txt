# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample3-delete-recover LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample3-delete-recover
  sample3_delete_recover.cpp
)

create_per_service_target_build(sample3-delete-recover keyvault)

target_link_libraries(sample3-delete-recover PRIVATE azure-security-keyvault-secrets azure-identity get-env-helper)

# Add the sample to be run on CI. 
# CI pipeline reads the {service}-samples.txt and runs the binaries listed there.
add_sample_for_ci_run(keyvault sample3-delete-recover)
