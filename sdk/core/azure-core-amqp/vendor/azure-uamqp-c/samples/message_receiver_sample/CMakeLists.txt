#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(message_receiver_sample
	main.c)

compileTargetAsC99(message_receiver_sample)

include_directories(.)
if (WIN32)
	#windows needs this define
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)

if(WINCE)
	target_link_libraries(message_receiver_sample
		uamqp aziotsharedutil secur32)
else()
	target_link_libraries(message_receiver_sample
		uamqp aziotsharedutil ws2_32 secur32)
endif()

	if(${use_openssl})
		target_link_libraries(message_receiver_sample
			$ENV{OpenSSLDir}/lib/ssleay32.lib $ENV{OpenSSLDir}/lib/libeay32.lib)

		file(COPY $ENV{OpenSSLDir}/bin/libeay32.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
		file(COPY $ENV{OpenSSLDir}/bin/ssleay32.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
	endif()
	if(${use_wolfssl})
		target_link_libraries(message_receiver_sample $ENV{WolfSSLDir}/Debug/wolfssl.lib)
	endif()
else()
	target_link_libraries(message_receiver_sample uamqp aziotsharedutil ${OPENSSL_LIBRARIES})
endif()
