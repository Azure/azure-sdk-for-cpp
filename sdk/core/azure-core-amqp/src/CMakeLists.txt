# Copyright(c) Microsoft Corporation.All rights reserved.
# SPDX - License - Identifier : MIT

# CMakeList.txt : CMake project for azure-core-amqp, include source and define
# project specific logic here.
#

find_package(uamqp CONFIG REQUIRED)
find_package(umock_c)
find_package(azure_macro_utils_c CONFIG REQUIRED)
find_package(azure_c_shared_utility CONFIG REQUIRED)

# Add source to this projects executable.
add_library(azure-core-amqp 
	AMQP/connection.cpp
	Models/amqp_value.cpp
	Models/amqp_header.cpp
	Models/amqp_message.cpp
	Models/amqp_properties.cpp
	Network/tls_transport.cpp
	Network/socket_transport.cpp 
	Network/socket_listener.cpp
	common/global_state.cpp
	Network/transport.cpp 
	AMQP/session.cpp
	Network/amqp_header_transport.cpp
	AMQP/message_receiver.cpp
	Models/messaging_values.cpp
	AMQP/link.cpp
	AMQP/message_sender.cpp
	AMQP/cancellable.cpp
	Network/sasl_transport.cpp
	Models/TransferInstance.cpp
	AMQP/connection_string_credential.cpp
	AMQP/claim_based_security.cpp
	Models/message_source.cpp
	Models/message_target.cpp
)

		
target_link_libraries(azure-core-amqp PRIVATE uamqp umock_c azure_macro_utils_c PUBLIC Azure::azure-core)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET azure-core-amqp PROPERTY CXX_STANDARD 14)
endif()
