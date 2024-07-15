# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (eventhubs-basic-operations LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

macro (define_sample samplename)
add_executable (
  eventhubs-${samplename}
  ${samplename}.cpp)

CREATE_PER_SERVICE_TARGET_BUILD_FOR_SAMPLE(eventhubs eventhubs-${samplename} FALSE)

target_link_libraries(eventhubs-${samplename} PRIVATE azure-messaging-eventhubs azure-identity)
endmacro()

define_sample(create_producer)
define_sample(create_consumer)
define_sample(create_consumer_aad)
define_sample(create_producer_aad)
