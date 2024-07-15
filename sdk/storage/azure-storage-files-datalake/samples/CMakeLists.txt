# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(datalake-getting-started datalake_getting_started.cpp)
create_per_service_target_build_for_sample(storage datalake-getting-started TRUE)
target_link_libraries(datalake-getting-started PRIVATE azure-storage-files-datalake get-env-helper)
