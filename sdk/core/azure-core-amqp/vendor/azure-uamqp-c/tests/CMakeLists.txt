#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

usePermissiveRulesForSamplesAndTests()

include_directories(.)

add_subdirectory(amqp_frame_codec_ut)
add_subdirectory(amqpvalue_ut)
add_subdirectory(amqp_management_ut)
add_subdirectory(async_operation_ut)
add_subdirectory(cbs_ut)
add_subdirectory(connection_ut)
add_subdirectory(frame_codec_ut)
add_subdirectory(header_detect_io_ut)
add_subdirectory(message_ut)
add_subdirectory(sasl_anonymous_ut)
add_subdirectory(sasl_frame_codec_ut)
add_subdirectory(sasl_mechanism_ut)
add_subdirectory(sasl_plain_ut)
add_subdirectory(sasl_server_mechanism_ut)
add_subdirectory(session_ut)
add_subdirectory(saslclientio_ut)

if(${run_e2e_tests})
    if(${use_socketio})
        add_subdirectory(local_client_server_tcp_e2e)
    endif()
    add_subdirectory(iothub_e2e)
endif()

add_subdirectory(local_client_server_tcp_perf)
