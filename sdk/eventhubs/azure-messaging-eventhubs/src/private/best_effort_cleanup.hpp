// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <exception>

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {

  // Cleanup must continue after one resource reports that its connection is already gone.
  template <class Iterator, class Action, class ErrorHandler>
  void ForEachBestEffort(
      Iterator first,
      Iterator last,
      Action const& action,
      ErrorHandler const& onError)
  {
    for (; first != last; ++first)
    {
      try
      {
        action(*first);
      }
      catch (std::exception const& ex)
      {
        onError(*first, ex);
      }
    }
  }

}}}} // namespace Azure::Messaging::EventHubs::_detail
