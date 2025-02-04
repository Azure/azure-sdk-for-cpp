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

CREATE_PER_SERVICE_TARGET_BUILD_FOR_SAMPLE(eventhubs eventhubs-${samplename})

target_link_libraries(eventhubs-${samplename} PRIVATE azure-messaging-eventhubs azure-identity)
target_compile_definitions(eventhubs-${samplename} PRIVATE _azure_BUILDING_SAMPLES)
endmacro()

define_sample(create_consumer_aad)
define_sample(create_producer_aad)
