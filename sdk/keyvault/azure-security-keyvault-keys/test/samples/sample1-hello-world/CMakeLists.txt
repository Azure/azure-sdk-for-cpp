# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample1-hello-world LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample1-hello-world
  sample1_hello_world.cpp
)

create_per_service_target_build(sample1-hello-world keyvault)

# Add the sample to be run on CI. 
# CI pipeline reads the {service}-samples.txt and runs the binaries listed there.
add_sample_for_ci_run(keyvault sample1-hello-world)

target_link_libraries(sample1-hello-world PRIVATE azure-security-keyvault-keys azure-identity get-env-helper)
