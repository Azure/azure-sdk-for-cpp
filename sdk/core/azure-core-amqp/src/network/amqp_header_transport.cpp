// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/network/amqp_header_detect_transport.hpp"

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/socketio.h>
#include <azure_uamqp_c/header_detect_io.h>
#include <azure_uamqp_c/socket_listener.h>
#include <exception>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  AmqpHeaderTransport::AmqpHeaderTransport(
      XIO_HANDLE parentTransport,
      TransportEvents* eventHandler)
      : Transport(eventHandler)
  {
    HEADER_DETECT_IO_CONFIG detectIoConfig{};
    HEADER_DETECT_ENTRY headerDetectEntries[] = {
        //        {header_detect_io_get_sasl_amqp_header(), nullptr},
        {header_detect_io_get_amqp_header(), nullptr},
    };
    detectIoConfig.underlying_io = parentTransport;
    detectIoConfig.header_detect_entry_count = std::extent<decltype(headerDetectEntries)>::value;
    detectIoConfig.header_detect_entries = headerDetectEntries;
    auto xio = xio_create(header_detect_io_get_interface_description(), &detectIoConfig);

    SetInstance(xio);
  }

}}}}} // namespace Azure::Core::Amqp::Network::_internal
