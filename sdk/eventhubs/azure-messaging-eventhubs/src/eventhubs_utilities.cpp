// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/eventhubs_utilities.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {
  namespace {

    size_t LogRawData(std::ostream& os, size_t startOffset, const uint8_t* const pb, size_t cb)
    {
      // scratch buffer which will hold the data being logged.
      std::stringstream ss;

      size_t bytesToWrite = (cb < 0x10 ? cb : 0x10);

      ss << std::hex << std::right << std::setw(8) << std::setfill('0') << startOffset << ": ";

      // Write the buffer data out.
      for (size_t i = 0; i < bytesToWrite; i += 1)
      {
        ss << std::hex << std::right << std::setw(2) << std::setfill('0') << static_cast<int>(pb[i])
           << " ";
      }

      // Now write the data in string format (similar to what the debugger does).
      // Start by padding partial lines to a fixed end.
      for (size_t i = bytesToWrite; i < 0x10; i += 1)
      {
        ss << "   ";
      }
      ss << "  * ";
      for (size_t i = 0; i < bytesToWrite; i += 1)
      {
        if (isprint(pb[i]))
        {
          ss << pb[i];
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

      ss << " *";

      os << ss.str();

      return bytesToWrite;
    }
  } // namespace
  void EventHubsUtilities::LogRawBuffer(std::ostream& os, std::vector<uint8_t> const& value)
  {
    const uint8_t* pb = value.data();
    size_t cb = value.size();
    size_t currentOffset = 0;
    do
    {
      auto cbLogged = LogRawData(os, currentOffset, pb, cb);
      pb += cbLogged;
      cb -= cbLogged;
      currentOffset += cbLogged;
      if (cb)
      {
        os << std::endl;
      }
    } while (cb);
  }
}}}} // namespace Azure::Messaging::EventHubs::_detail
