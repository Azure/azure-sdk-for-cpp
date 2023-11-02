// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/network/amqp_header_detect_transport.hpp"
#include "private/transport_impl.hpp"

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/socketio.h>
#include <azure_uamqp_c/header_detect_io.h>
#include <azure_uamqp_c/socket_listener.h>

#include <exception>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  Transport AmqpHeaderDetectTransportFactory::Create(
      std::shared_ptr<Transport> parentTransport,
      TransportEvents* eventHandler)
  {
    HEADER_DETECT_IO_CONFIG detectIoConfig{};
    HEADER_DETECT_ENTRY headerDetectEntries[] = {
        //        {header_detect_io_get_sasl_amqp_header(), nullptr},
        {header_detect_io_get_amqp_header(), nullptr},
    };
    detectIoConfig.underlying_io = parentTransport->GetImpl()->Release();
    detectIoConfig.header_detect_entry_count = std::extent<decltype(headerDetectEntries)>::value;
    detectIoConfig.header_detect_entries = headerDetectEntries;
    return _detail::TransportImpl::CreateFromXioHandle(
        xio_create(header_detect_io_get_interface_description(), &detectIoConfig), eventHandler);
  }

}}}}} // namespace Azure::Core::Amqp::Network::_internal
