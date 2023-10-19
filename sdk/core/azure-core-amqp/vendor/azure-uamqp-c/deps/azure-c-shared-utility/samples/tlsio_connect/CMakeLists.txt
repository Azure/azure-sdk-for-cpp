#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

generate_cpp_wrapper(tlsio_connect_c_files main)

IF(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
ENDIF(WIN32)

add_executable(tlsio_connect ${tlsio_connect_c_files})

target_link_libraries(tlsio_connect
    aziotsharedutil
)

if(${use_openssl} AND WIN32)
	file(COPY ${SSL_DLL} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	file(COPY ${CRYPTO_DLL} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

set_target_properties(tlsio_connect
    PROPERTIES
    FOLDER "azure_c_shared_utility_samples")

compileTargetAsC99(tlsio_connect)
