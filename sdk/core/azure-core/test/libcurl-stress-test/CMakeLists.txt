# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.13)

project(azure-core-libcurl-stress-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(
  azure-core-libcurl-stress-test
  libcurl_stress_test.cpp
)

target_link_libraries(azure-core-libcurl-stress-test PRIVATE azure-core)

create_map_file(azure-core-libcurl-stress-test azure-core-libcurl-stress-test.map)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Dockerfile
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

find_program(DOCKER_EXECUTABLE docker)
#if docker is found try to build the docker image in the defined docker_build stage which should be run after build
if(DOCKER_EXECUTABLE)
  add_custom_target(docker_build
    COMMAND ${DOCKER_EXECUTABLE} build --build-arg targetTest=azure-core-libcurl-stress-test --build-arg build=on --tag=gearama/curlstress .
  )
endif()