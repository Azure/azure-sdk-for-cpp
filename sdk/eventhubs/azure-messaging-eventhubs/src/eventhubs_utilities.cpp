// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/eventhubs_utilities.hpp"

#include <azure/core/amqp/models/amqp_error.hpp>

#include <iomanip>
#include <iostream>
#include <sstream>

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
    bool isTransient = false;
    if ((condition == AmqpErrorCondition::TimeoutError)
        || (condition == AmqpErrorCondition::ServerBusyError)
        || (condition == AmqpErrorCondition::InternalError)
        || (condition == AmqpErrorCondition::LinkDetachForced)
        || (condition == AmqpErrorCondition::ConnectionForced)
        || (condition == AmqpErrorCondition::ConnectionFramingError)
        || (condition == AmqpErrorCondition::ProtonIo))
    {
      isTransient = true;
    }
    else if (condition == AmqpErrorCondition::NotFound)
    {
      // Note: Java has additional processing here, it looks for the regex:
      // "The messaging entity .* could not be found" in the error discription and if it is
      // found it treats the error as not transient. For now, just treat NotFound as transient.
      isTransient = true;
    }
    return isTransient;
  }
}}}} // namespace Azure::Messaging::EventHubs::_detail
