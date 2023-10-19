#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(websockets_sample
    main.c
    iothub_certs.h
    iothub_certs.c)

compileTargetAsC99(websockets_sample)

include_directories(.)

if(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)

if(WINCE)
	target_link_libraries(websockets_sample
		uamqp aziotsharedutil secur32)
else()
	target_link_libraries(websockets_sample
		uamqp aziotsharedutil ws2_32 secur32)
endif()

else()
    target_link_libraries(websockets_sample	uamqp aziotsharedutil)
endif()
