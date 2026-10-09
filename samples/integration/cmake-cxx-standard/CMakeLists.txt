# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)
project(cmake-cxx-standard-tests LANGUAGES NONE)
enable_testing()

set(configure_args -G "${CMAKE_GENERATOR}")
foreach(variable CMAKE_GENERATOR_PLATFORM CMAKE_GENERATOR_TOOLSET CMAKE_TOOLCHAIN_FILE CMAKE_PREFIX_PATH)
  if(DEFINED ${variable} AND NOT "${${variable}}" STREQUAL "")
    string(REPLACE ";" "\\;" value "${${variable}}")
    list(APPEND configure_args "-D${variable}=${value}")
  endif()
endforeach()

foreach(surface service package)
  foreach(standard default 98 11 14 17 20 parent17 parent20)
    set(standard_args)
    set(expected_standard 14)
    if(standard MATCHES "^parent(17|20)$")
      set(expected_standard "${CMAKE_MATCH_1}")
      list(APPEND standard_args "-DTEST_PARENT_STANDARD=${expected_standard}")
    elseif(NOT standard STREQUAL "default")
      list(APPEND standard_args "-DCMAKE_CXX_STANDARD=${standard}")
      if(standard GREATER 14 AND NOT standard STREQUAL "98")
        set(expected_standard "${standard}")
      endif()
    endif()

    add_test(
      NAME cxx-standard-${surface}-${standard}
      COMMAND "${CMAKE_COMMAND}"
        -S "${CMAKE_CURRENT_SOURCE_DIR}/project"
        -B "${CMAKE_CURRENT_BINARY_DIR}/${surface}-${standard}"
        ${configure_args}
        ${standard_args}
        "-DTEST_SURFACE=${surface}"
        "-DTEST_EXPECTED_STANDARD=${expected_standard}"
    )
  endforeach()
endforeach()
