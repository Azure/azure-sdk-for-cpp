# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# The option is added again just like from the main CMakeList
# to enable discovering the option directly from each project.
option(BUILD_RTTI "Build libraries with run-time type information." ON)

macro(az_remove_rtti)
  if (BUILD_RTTI)
    add_compile_definitions(AZURE_SDK_RTTI_ENABLED)
  else()
    message("Building library without RTTI")
  endif()
endmacro()
