# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(queue-getting-started queue_getting_started.cpp)
target_link_libraries(queue-getting-started PRIVATE azure-storage-queues get-env-helper)
create_per_service_target_build_for_sample(storage queue-getting-started)
target_compile_definitions(queue-getting-started PRIVATE _azure_BUILDING_SAMPLES)

add_executable(queue-encode-message queue_encode_message.cpp)
target_link_libraries(queue-encode-message PRIVATE azure-storage-queues get-env-helper)
create_per_service_target_build_for_sample(storage queue-encode-message)
target_compile_definitions(queue-encode-message PRIVATE _azure_BUILDING_SAMPLES)
