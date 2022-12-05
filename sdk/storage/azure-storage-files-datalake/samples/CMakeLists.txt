# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

add_executable(datalake-getting-started datalake_getting_started.cpp)
create_per_service_target_build_for_sample(storage datalake-getting-started)
target_link_libraries(datalake-getting-started PRIVATE azure-storage-files-datalake get-env-helper)