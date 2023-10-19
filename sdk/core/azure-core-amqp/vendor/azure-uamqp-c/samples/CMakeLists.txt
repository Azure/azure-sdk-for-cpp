#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

usePermissiveRulesForSamplesAndTests()

function(add_sample_directory whatIsBuilding)
    add_subdirectory(${whatIsBuilding})

    set_target_properties(${whatIsBuilding}
               PROPERTIES
               FOLDER "uamqp_samples")
endfunction()

add_sample_directory(message_sender_sample)
add_sample_directory(message_receiver_sample)
add_sample_directory(mssbcbs_sample)
add_sample_directory(eh_sender_with_sas_token_sample)
add_sample_directory(local_client_sample)

if(WIN32)
    add_sample_directory(local_server_sample)
if(NOT WINCE)
    add_sample_directory(local_server_tls_sample)
endif()
endif()

add_sample_directory(websockets_sample)
