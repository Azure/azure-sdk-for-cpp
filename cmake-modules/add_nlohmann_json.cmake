# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Defines utility functions to create code coverage targets with gcov.
# gcov html and xml report.
#

# Azure Core can expose the Nlohmann Json header by fetching a release version and including the header to the include path.
# 
# Azure Core will try to find the json header first. This is for an application that is already using Nlohmann header.
# When it is not found it, Azure Core will featch it.

# Storage requires 3.8 version for using `contains` feature
find_package(nlohmann_json 3.8.0 CONFIG)
if (NOT nlohmann_json_FOUND)

    include(FetchContent)

    # realese code only. This save us from getting the entire nlohmann source code.
    FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
        GIT_TAG v3.8.0)

    FetchContent_GetProperties(json)
    if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()

endif()
