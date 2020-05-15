# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.12)

if (MSVC)
    # There will be many const conparisons in test code.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd6326")
endif()

add_executable (
     azure-storage-test
     test_base.hpp
     test_base.cpp
     block_blob_client_test.cpp 
     main.cpp 
)

target_link_libraries(azure-storage-test PRIVATE azure-storage)

add_gtest(azure-storage-test)
