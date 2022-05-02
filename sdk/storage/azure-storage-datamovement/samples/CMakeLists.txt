# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

add_executable(datamovement-getting-started datamovement_getting_started.cpp)
target_link_libraries(datamovement-getting-started PRIVATE azure-storage-datamovement get-env-helper)
create_per_service_target_build_for_sample(storage datamovement-getting-started)
