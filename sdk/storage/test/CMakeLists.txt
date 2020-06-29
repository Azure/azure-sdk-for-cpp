# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.15)

add_executable (
     azure-storage-test
     test_base.hpp
     test_base.cpp
     blob_service_client_test.cpp
     blob_container_client_test.hpp
     blob_container_client_test.cpp
     block_blob_client_test.hpp
     block_blob_client_test.cpp
     append_blob_client_test.hpp
     append_blob_client_test.cpp
     page_blob_client_test.hpp
     page_blob_client_test.cpp
     main.cpp 
)

target_link_libraries(azure-storage-test PRIVATE azure-storage)

if (MSVC)
    target_compile_options(azure-storage-test PRIVATE /wd6326 /wd26495 /wd26812)
endif()

add_gtest(azure-storage-test)
