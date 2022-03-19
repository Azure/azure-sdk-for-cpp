# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample6-wrap-unwrap LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample6-wrap-unwrap
  sample6_wrap_unwrap.cpp
)
create_per_service_target_build(sample6-wrap-unwrap keyvault)

# Add the sample to be run on CI. 
# CI pipeline reads the {service}-samples.txt and runs the binaries listed there.
add_sample_for_ci_run(keyvault sample6-wrap-unwrap)

target_link_libraries(sample6-wrap-unwrap PRIVATE azure-security-keyvault-keys azure-identity get-env-helper)
