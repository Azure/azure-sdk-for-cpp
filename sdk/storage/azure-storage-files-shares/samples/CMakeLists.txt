# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(file-share-getting-started file_share_getting_started.cpp)
create_per_service_target_build_for_sample(storage file-share-getting-started TRUE)
target_link_libraries(file-share-getting-started PRIVATE azure-storage-files-shares get-env-helper)
