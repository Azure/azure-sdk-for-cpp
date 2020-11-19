# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

target_sources(
    azure-storage-sample
    PRIVATE
    main.cpp
    samples_common.hpp
)

target_include_directories(azure-storage-sample PUBLIC .)
