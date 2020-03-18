# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.12)

project (azure-core LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library (
  azure-core
  src/http/az_http_request
  )

if(MSVC)
  if(WARNINGS_AS_ERRORS)
    set(WARNINGS_AS_ERRORS_FLAG "/WX")
  endif()

  target_compile_options(azure-core PRIVATE /Wall ${WARNINGS_AS_ERRORS_FLAG} /wd5031 /wd4668 /wd4820 /wd4255 /analyze)
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
  if(WARNINGS_AS_ERRORS)
    set(WARNINGS_AS_ERRORS_FLAG "-Werror")
  endif()

  target_compile_options(azure-core PRIVATE -Xclang -Wall -Wextra -pedantic  ${WARNINGS_AS_ERRORS_FLAG} -Wdocumentation -Wdocumentation-unknown-command -Wcast-qual)
else()
  if(WARNINGS_AS_ERRORS)
    set(WARNINGS_AS_ERRORS_FLAG "-Werror")
  endif()

  target_compile_options(azure-core PRIVATE -Wall -Wextra -pedantic  ${WARNINGS_AS_ERRORS_FLAG})
endif()

target_include_directories (azure-core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include/az_core>)
target_include_directories (azure-core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/internal> $<INSTALL_INTERFACE:include/az_core_internal>)

# make sure that users can consume the project as a library.
add_library (azure::core ALIAS azure-core)
