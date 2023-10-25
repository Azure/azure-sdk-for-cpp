#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.5)
project(uamqp)

FILE(READ ${CMAKE_CURRENT_LIST_DIR}/version.txt UAMQP_VERSION)

option(run_e2e_tests "set run_e2e_tests to ON to run e2e tests (default is OFF) [if possible, they are always built]" OFF)
option(run_unittests "set run_unittests to ON to run unittests (default is OFF)" OFF)
option(skip_samples "set skip_samples to ON to skip building samples (default is OFF)[if possible, they are always built]" OFF)
option(use_installed_dependencies "set use_installed_dependencies to ON to use installed packages instead of building dependencies from submodules" OFF)
option(memory_trace "set memory_trace to ON if memory usage is to be used, set to OFF to not use it" OFF)
option(use_custom_heap "use externally defined heap functions instead of the malloc family" OFF)

if(${use_custom_heap})
    add_definitions(-DGB_USE_CUSTOM_HEAP)
endif()

if(${no_logging})
    add_definitions(-DNO_LOGGING)
endif()

#do not add or build any tests of the dependencies
set(original_run_e2e_tests ${run_e2e_tests})
set(original_run_int_tests ${run_int_tests})
set(original_run_unittests ${run_unittests})

set(run_e2e_tests OFF)
set(run_int_tests OFF)
set(run_unittests OFF)

if(NOT ${use_installed_dependencies})
    if ((NOT TARGET azure_macro_utils_c) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/azure-macro-utils-c/CMakeLists.txt))
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-macro-utils-c)
    endif()
    if ((NOT TARGET umock_c) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/umock-c/CMakeLists.txt))
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/umock-c)
    endif()

    if(${original_run_e2e_tests} OR ${original_run_unittests})
        if ((NOT TARGET testrunnerswitcher) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-testrunnerswitcher/CMakeLists.txt))
            add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-testrunnerswitcher)
        endif()
        if ((NOT TARGET ctest) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/azure-ctest/CMakeLists.txt))
            add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-ctest)
        endif()
    endif()
    if ((NOT TARGET aziotsharedutil) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-shared-utility/CMakeLists.txt))
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-shared-utility)
    endif()
else()
    if (NOT azure_macro_utils_cFOUND)
        find_package(azure_macro_utils_c REQUIRED CONFIG)
    endif ()
    if (NOT umock_cFOUND)
        find_package(umock_c REQUIRED CONFIG)
    endif ()
    if (NOT azure_c_shared_utilityFOUND)
        find_package(azure_c_shared_utility REQUIRED CONFIG)
    endif ()
    
    include(${azure_c_shared_utility_DIR}/azure_c_shared_utilityConfig.cmake)
    include(${azure_c_shared_utility_DIR}/azure_c_shared_utilityFunctions.cmake)
    include(${azure_c_shared_utility_DIR}/azure_iot_build_rules.cmake)
endif()

include_directories(${MACRO_UTILS_INC_FOLDER} ${UMOCK_C_INC_FOLDER})

# Include the common build rules for C shared utility
if(NOT ${use_installed_dependencies})
    include(${SHARED_UTIL_FOLDER}/configs/azure_iot_build_rules.cmake)

    set_platform_files(${SHARED_UTIL_FOLDER})
endif()


set(run_e2e_tests ${original_run_e2e_tests})
set(run_int_tests ${original_run_int_tests})
set(run_unittests ${original_run_unittests})

if(${memory_trace})
    add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
endif()

option(use_socketio "set use_socketio to ON if socketio is to be included in the library, set to OFF if a different implementation will be provided" ON)
if(WIN32)
    option(use_schannel "set use_schannel to ON if schannel is to be used, set to OFF to not use schannel" ON)
    option(use_openssl "set use_openssl to ON if openssl is to be used, set to OFF to not use openssl" OFF)
    option(use_wolfssl "set use_wolfssl to ON if wolfssl is to be used, set to OFF to not use wolfssl" OFF)
else()
    option(use_schannel "set use_schannel to ON if schannel is to be used, set to OFF to not use schannel" OFF)
    option(use_openssl "set use_openssl to ON if openssl is to be used, set to OFF to not use openssl" ON)
    option(use_wolfssl "set use_wolfssl to ON if wolfssl is to be used, set to OFF to not use wolfssl" OFF)
endif()

# The native apple tlsio does not support socket_io
if(MACOSX AND NOT ${use_openssl})
    set(use_socketio OFF)
endif()

add_definitions(-DREFCOUNT_ATOMIC_DONTCARE)
add_definitions(-D__STDC_NO_ATOMICS__=1)


include_directories(${CMAKE_CURRENT_LIST_DIR}/inc)

if(WIN32)
    include_directories($ENV{OpenSSLDir}/include)
endif()

set(uamqp_h_files
    ./inc/azure_uamqp_c/amqp_definitions_role.h
    ./inc/azure_uamqp_c/amqp_definitions_sender_settle_mode.h
    ./inc/azure_uamqp_c/amqp_definitions_receiver_settle_mode.h
    ./inc/azure_uamqp_c/amqp_definitions_handle.h
    ./inc/azure_uamqp_c/amqp_definitions_seconds.h
    ./inc/azure_uamqp_c/amqp_definitions_milliseconds.h
    ./inc/azure_uamqp_c/amqp_definitions_delivery_tag.h
    ./inc/azure_uamqp_c/amqp_definitions_sequence_no.h
    ./inc/azure_uamqp_c/amqp_definitions_delivery_number.h
    ./inc/azure_uamqp_c/amqp_definitions_transfer_number.h
    ./inc/azure_uamqp_c/amqp_definitions_message_format.h
    ./inc/azure_uamqp_c/amqp_definitions_ietf_language_tag.h
    ./inc/azure_uamqp_c/amqp_definitions_fields.h
    ./inc/azure_uamqp_c/amqp_definitions_error.h
    ./inc/azure_uamqp_c/amqp_definitions_amqp_error.h
    ./inc/azure_uamqp_c/amqp_definitions_connection_error.h
    ./inc/azure_uamqp_c/amqp_definitions_session_error.h
    ./inc/azure_uamqp_c/amqp_definitions_link_error.h
    ./inc/azure_uamqp_c/amqp_definitions_open.h
    ./inc/azure_uamqp_c/amqp_definitions_begin.h
    ./inc/azure_uamqp_c/amqp_definitions_attach.h
    ./inc/azure_uamqp_c/amqp_definitions_flow.h
    ./inc/azure_uamqp_c/amqp_definitions_transfer.h
    ./inc/azure_uamqp_c/amqp_definitions_disposition.h
    ./inc/azure_uamqp_c/amqp_definitions_detach.h
    ./inc/azure_uamqp_c/amqp_definitions_end.h
    ./inc/azure_uamqp_c/amqp_definitions_close.h
    ./inc/azure_uamqp_c/amqp_definitions_sasl_code.h
    ./inc/azure_uamqp_c/amqp_definitions_sasl_mechanisms.h
    ./inc/azure_uamqp_c/amqp_definitions_sasl_init.h
    ./inc/azure_uamqp_c/amqp_definitions_sasl_challenge.h
    ./inc/azure_uamqp_c/amqp_definitions_sasl_response.h
    ./inc/azure_uamqp_c/amqp_definitions_sasl_outcome.h
    ./inc/azure_uamqp_c/amqp_definitions_terminus_durability.h
    ./inc/azure_uamqp_c/amqp_definitions_terminus_expiry_policy.h
    ./inc/azure_uamqp_c/amqp_definitions_node_properties.h
    ./inc/azure_uamqp_c/amqp_definitions_filter_set.h
    ./inc/azure_uamqp_c/amqp_definitions_source.h
    ./inc/azure_uamqp_c/amqp_definitions_target.h
    ./inc/azure_uamqp_c/amqp_definitions_annotations.h
    ./inc/azure_uamqp_c/amqp_definitions_message_id_ulong.h
    ./inc/azure_uamqp_c/amqp_definitions_message_id_uuid.h
    ./inc/azure_uamqp_c/amqp_definitions_message_id_binary.h
    ./inc/azure_uamqp_c/amqp_definitions_message_id_string.h
    ./inc/azure_uamqp_c/amqp_definitions_address_string.h
    ./inc/azure_uamqp_c/amqp_definitions_header.h
    ./inc/azure_uamqp_c/amqp_definitions_delivery_annotations.h
    ./inc/azure_uamqp_c/amqp_definitions_message_annotations.h
    ./inc/azure_uamqp_c/amqp_definitions_application_properties.h
    ./inc/azure_uamqp_c/amqp_definitions_data.h
    ./inc/azure_uamqp_c/amqp_definitions_amqp_sequence.h
    ./inc/azure_uamqp_c/amqp_definitions_amqp_value.h
    ./inc/azure_uamqp_c/amqp_definitions_footer.h
    ./inc/azure_uamqp_c/amqp_definitions_properties.h
    ./inc/azure_uamqp_c/amqp_definitions_received.h
    ./inc/azure_uamqp_c/amqp_definitions_accepted.h
    ./inc/azure_uamqp_c/amqp_definitions_rejected.h
    ./inc/azure_uamqp_c/amqp_definitions_released.h
    ./inc/azure_uamqp_c/amqp_definitions_modified.h
    ./inc/azure_uamqp_c/amqp_definitions.h
    ./inc/azure_uamqp_c/amqp_frame_codec.h
    ./inc/azure_uamqp_c/amqp_management.h
    ./inc/azure_uamqp_c/amqp_types.h
    ./inc/azure_uamqp_c/amqpvalue.h
    ./inc/azure_uamqp_c/amqpvalue_to_string.h
    ./inc/azure_uamqp_c/async_operation.h
    ./inc/azure_uamqp_c/cbs.h
    ./inc/azure_uamqp_c/connection.h
    ./inc/azure_uamqp_c/frame_codec.h
    ./inc/azure_uamqp_c/header_detect_io.h
    ./inc/azure_uamqp_c/link.h
    ./inc/azure_uamqp_c/message.h
    ./inc/azure_uamqp_c/message_receiver.h
    ./inc/azure_uamqp_c/message_sender.h
    ./inc/azure_uamqp_c/messaging.h
    ./inc/azure_uamqp_c/sasl_anonymous.h
    ./inc/azure_uamqp_c/sasl_frame_codec.h
    ./inc/azure_uamqp_c/sasl_mechanism.h
    ./inc/azure_uamqp_c/sasl_server_mechanism.h
    ./inc/azure_uamqp_c/sasl_mssbcbs.h
    ./inc/azure_uamqp_c/sasl_plain.h
    ./inc/azure_uamqp_c/saslclientio.h
    ./inc/azure_uamqp_c/sasl_server_io.h
    ./inc/azure_uamqp_c/server_protocol_io.h
    ./inc/azure_uamqp_c/session.h
    ./inc/azure_uamqp_c/socket_listener.h
    ./inc/azure_uamqp_c/uamqp.h
)

set(uamqp_c_files
    ./src/amqp_definitions.c
    ./src/amqp_frame_codec.c
    ./src/amqp_management.c
    ./src/amqpvalue.c
    ./src/amqpvalue_to_string.c
    ./src/async_operation.c
    ./src/cbs.c
    ./src/connection.c
    ./src/frame_codec.c
    ./src/header_detect_io.c
    ./src/link.c
    ./src/message.c
    ./src/message_receiver.c
    ./src/message_sender.c
    ./src/messaging.c
    ./src/sasl_anonymous.c
    ./src/sasl_frame_codec.c
    ./src/sasl_mechanism.c
    ./src/sasl_server_mechanism.c
    ./src/sasl_mssbcbs.c
    ./src/sasl_plain.c
    ./src/saslclientio.c
    ./src/session.c
)

if(WIN32)
    set(socketlistener_c_files
        ./src/socket_listener_win32.c
    )
elseif(UNIX)
    set(socketlistener_c_files
        ./src/socket_listener_berkeley.c
    )
else()
    set(socketlistener_c_files
    )
endif()

add_library(uamqp
    ${uamqp_c_files}
    ${uamqp_h_files}
    ${socketlistener_c_files}
    )
setTargetBuildProperties(uamqp)

target_link_libraries(uamqp aziotsharedutil)

if (NOT ${skip_samples})
    add_subdirectory(samples)
endif()

if (${run_unittests})
    include("dependencies-test.cmake")
    add_subdirectory(tests)
endif()

# The following "set" statetement exports across the project a global variable
set(UAMQP_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "This is the include folder for UAMQP" FORCE)
set(UAMQP_SRC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/src CACHE INTERNAL "This is the lib folder for UAMQP" FORCE)

# Set CMAKE_INSTALL_LIBDIR if not defined
include(GNUInstallDirs)

# Install uamqp
set(package_location "cmake")

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR "lib")
endif()

install(TARGETS uamqp EXPORT uamqpTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/../bin
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/azureiot
)
install(FILES ${uamqp_h_files} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/azureiot/azure_uamqp_c)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${UAMQP_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_file("configs/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
    COPYONLY
)

install(EXPORT uamqpTargets
    FILE
        "${PROJECT_NAME}Targets.cmake"
    DESTINATION
        ${package_location}
)

install(
    FILES
        "configs/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION
        ${package_location}
)

compileTargetAsC99(uamqp)
