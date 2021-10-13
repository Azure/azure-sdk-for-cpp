# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# The option is added again just like from the main CMakeList
# to enable discovering the option directly from each project.
option(BUILD_RTTI "Build libraries with Runtime type information." ON)

macro(az_remove_rtti target)
  if (BUILD_RTTI)
    add_compile_definitions(AZURE_SDK_RTTI_ENABLED)
  else()
    message("Building library ${target} without RTTI")
    if(MSVC)
      target_compile_options(${target} PRIVATE /GR-)
    else()
      target_compile_options(${target} PRIVATE -fno-rtti)
    endif()
  endif()
endmacro()
