# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (azure-messaging LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

message(STATUS "Detecting compiler.")
if (CMAKE_CXX_COMPILER MATCHES "clang")
  if($ENV{CXX} MATCHES "-1[12]")
    message(STATUS "Disabling test discovery in EventHubs library for CLANG-11 and CLANG-12 builds because the eventhubs tests segfault for unknown reasons.")
    set(DISABLE_TEST_DISCOVERY ON CACHE BOOL "Disable test discovery for library.")
  endif()
endif()

add_subdirectory(azure-messaging-eventhubs)
add_subdirectory(azure-messaging-eventhubs-checkpointstore-blob)
