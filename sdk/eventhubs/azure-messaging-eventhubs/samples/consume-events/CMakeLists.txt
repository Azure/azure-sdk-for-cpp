# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (eventhubs-consume-events LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

macro (define_sample samplename)
add_executable (
  eventhubs-${samplename}
  ${samplename}.cpp)

CREATE_PER_SERVICE_TARGET_BUILD_FOR_SAMPLE(eventhubs eventhubs-${samplename} DISABLE_RUN)

target_link_libraries(eventhubs-${samplename} PRIVATE azure-messaging-eventhubs azure-identity)
endmacro()

define_sample(consume_events)
define_sample(consume_events_aad)
