// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/eventhubs_utilities.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

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
}}}} // namespace Azure::Messaging::EventHubs::_detail
