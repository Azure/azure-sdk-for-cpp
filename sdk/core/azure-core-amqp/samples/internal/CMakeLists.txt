# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.13)

# Include sub-projects.
if(USE_UAMQP)
add_subdirectory ("local_server_sample")
endif()
add_subdirectory ("local_client_sample")
add_subdirectory ("local_client_async_sample")
#add_subdirectory ("eventhub_writer_sample")
add_subdirectory ("eventhub_sas_writer_sample")
add_subdirectory ("eventhub_token_reader_sample")
add_subdirectory ("eventhub_token_writer_sample")
#add_subdirectory ("eventhub_async_writer_sample")
#add_subdirectory ("eventhub_reader_sample")
add_subdirectory ("eventhub_sas_reader_sample")
add_subdirectory ("eventhub_get_properties_sample")
