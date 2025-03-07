#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(local_client_server_tcp_perf
	local_client_server_tcp_perf.c)

compileTargetAsC99(local_client_server_tcp_perf)

set_target_properties(local_client_server_tcp_perf
           PROPERTIES
           FOLDER "tests/uamqp_tests/perf")

if(WIN32)
	#windows needs this define
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	target_link_libraries(local_client_server_tcp_perf
		uamqp
		aziotsharedutil
		ws2_32
		secur32)

	if(${use_openssl})
		target_link_libraries(local_client_server_tcp_perf
			$ENV{OpenSSLDir}/lib/ssleay32.lib $ENV{OpenSSLDir}/lib/libeay32.lib)

		file(COPY $ENV{OpenSSLDir}/bin/libeay32.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
		file(COPY $ENV{OpenSSLDir}/bin/ssleay32.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
	endif()
	if(${use_wolfssl})
		target_link_libraries(local_client_server_tcp_perf $ENV{WolfSSLDir}/Debug/wolfssl.lib)
	endif()
else()
	target_link_libraries(local_client_server_tcp_perf uamqp aziotsharedutil)
        target_link_libraries(local_client_server_tcp_perf ${OPENSSL_LIBRARIES})
endif()

