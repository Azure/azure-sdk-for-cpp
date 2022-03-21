# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

add_executable(queue-getting-started queue_getting_started.cpp)
target_link_libraries(queue-getting-started PRIVATE azure-storage-queues get-env-helper)
create_per_service_target_build_for_sample(storage queue-getting-started)

add_executable(queue-encode-message queue_encode_message.cpp)
target_link_libraries(queue-encode-message PRIVATE azure-storage-queues get-env-helper)
create_per_service_target_build_for_sample(storage queue-encode-message)
