# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.12)

project (azure-core LANGUAGES CXX)

set(CMAKE_C_STANDARD 99)

add_library (
  azure-core
  src/az_span.c
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

target_include_directories (azure-core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include/az>)

# create interface for internal headers
#add_library(az_core_internal INTERFACE)
#target_include_directories(az_core_internal INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/internal> $<INSTALL_INTERFACE:include/az_internal>)

#target_link_libraries(az_core PRIVATE az_core_internal)

# make sure that users can consume the project as a library.
add_library (azure::core ALIAS azure-core)

 add_executable (
     azure-core_test
     test/main.cpp 
     )

 target_link_libraries(azure-core-test PRIVATE azure-core)
 target_include_directories (azure-core-test PRIVATE src)

 if (UNIX)
   target_link_libraries(azure-core-test PRIVATE m)
 endif()

 target_include_directories (azure-core-test PRIVATE test)

 add_test(NAME azure-core-test COMMAND azure-core-test)


if (MOCK_CURL)
  MESSAGE("Will mock all calls to http client with static responses")
  add_definitions(-DMOCK_CURL="MOCK")
endif()