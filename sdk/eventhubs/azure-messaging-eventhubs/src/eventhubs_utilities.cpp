// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/eventhubs_utilities.hpp"

#include "private/eventhubs_constants.hpp"

#include <azure/core/amqp/internal/connection_string_credential.hpp>
#include <azure/core/amqp/internal/models/amqp_error.hpp>
#include <azure/core/url.hpp>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace Azure::Core::Amqp::Models::_internal;

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {
  namespace {

    constexpr const size_t bytesPerLine = 0x10;

    size_t LogRawData(std::ostream& os, size_t startOffset, const uint8_t* const data, size_t count)
    {
      // scratch buffer which will hold the data being logged.
      std::stringstream ss;

      size_t bytesToWrite = (count < bytesPerLine ? count : bytesPerLine);

      ss << std::hex << std::right << std::setw(8) << std::setfill('0') << startOffset << ": ";

      // Write the buffer data out in hex.
      for (size_t i = 0; i < bytesToWrite; i += 1)
      {
        ss << std::hex << std::right << std::setw(2) << std::setfill('0')
           << static_cast<int>(data[i]) << " ";
      }

      // Now write the data in string format (similar to what the debugger does).
      // Start by padding partial lines to a fixed end.
      for (size_t i = bytesToWrite; i < bytesPerLine; i += 1)
      {
        ss << "   ";
      }
      // Start of text marker.
      ss << "  * ";
      for (size_t i = 0; i < bytesToWrite; i += 1)
      {
        if (isprint(data[i]))
        {
          ss << data[i];
        }
        else
        {

          ss << ".";
        }
      }
      for (size_t i = bytesToWrite; i < 0x10; i += 1)
      {
        ss << " ";
      }
      // End of text marker.
      ss << " *";

      os << ss.str();

      return bytesToWrite;
    }
  } // namespace

  ConnectionStringDetails EventHubsUtilities::CreateConnectionStringDetails(
      std::string const& connectionString,
      std::string const& eventHub)
  {
    auto sasCredential
        = std::make_shared<Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>(
            connectionString, eventHub);

    ConnectionStringDetails details;
    details.Credential = sasCredential;
    details.EventHub
        = sasCredential->GetEntityPath().empty() ? eventHub : sasCredential->GetEntityPath();
    if (details.EventHub.empty())
    {
      throw std::invalid_argument(
          "An Event Hub name is required when the connection string does not contain EntityPath.");
    }

    details.FullyQualifiedNamespace = sasCredential->GetHostName();
    details.Port = sasCredential->GetPort();
    details.ServiceScheme = EventHubsServiceScheme;

    if (sasCredential->UseDevelopmentEmulator())
    {
      details.ServiceScheme = EventHubsServiceScheme_Emulator;
      std::uint16_t const endpointPort{Azure::Core::Url(sasCredential->GetEndpoint()).GetPort()};
      if (endpointPort == Azure::Core::Amqp::_internal::AmqpTlsPort)
      {
        throw std::invalid_argument("The Event Hubs emulator cannot use the TLS AMQP port 5671.");
      }
      details.Port = endpointPort == 0 ? Azure::Core::Amqp::_internal::AmqpPort : endpointPort;
    }

    return details;
  }

  // Log the vector `value` in a structured format, bytesPerLine at a time.
  void EventHubsUtilities::LogRawBuffer(std::ostream& os, std::vector<uint8_t> const& value)
  {
    const uint8_t* data = value.data();
    size_t count = value.size();
    size_t currentOffset = 0;
    do
    {
      auto countLogged = LogRawData(os, currentOffset, data, count);
      data += countLogged;
      count -= countLogged;
      currentOffset += countLogged;
      if (count)
      {
        os << std::endl;
      }
    } while (count);
  }

  bool EventHubsExceptionFactory::IsErrorTransient(AmqpErrorCondition const& condition)
  {
    if (condition.ToString().empty())
    {
      return true;
    }

    static AmqpErrorCondition const TransientConditions[]
        = {AmqpErrorCondition::TimeoutError,
           AmqpErrorCondition::ServerBusyError,
           AmqpErrorCondition::InternalError,
           AmqpErrorCondition::LinkDetachForced,
           AmqpErrorCondition::ConnectionForced,
           AmqpErrorCondition::ConnectionFramingError,
           AmqpErrorCondition::ProtonIo,
           AmqpErrorCondition::NotFound,
           AmqpErrorCondition::IllegalState};

    for (auto const& transientCondition : TransientConditions)
    {
      if (condition == transientCondition)
      {
        return true;
      }
    }

    return false;
  }

}}}} // namespace Azure::Messaging::EventHubs::_detail
