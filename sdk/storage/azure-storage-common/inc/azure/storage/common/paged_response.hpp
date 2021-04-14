// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdlib>
#include <string>

#include <azure/core/context.hpp>
#include <azure/core/http/raw_response.hpp>

namespace Azure {

template <class Derived> class PagedResponse {
public:
  std::string NextPageToken;
  std::string CurrentPageToken;
  std::unique_ptr<Azure::Core::Http::RawResponse> RawResponse;

  bool HasMore() const { return !NextPageToken.empty(); }

  void NextPage(const Azure::Core::Context& context)
  {
    static_assert(
        std::is_base_of<PagedResponse, Derived>::value,
        "The template argument \"Derived\" should derive from PagedResponse<Derived>.");

    if (!HasMore())
    {
      // User should always check HasMore() before calling this function.
      std::abort();
    }
    CurrentPageToken = NextPageToken;
    static_cast<Derived*>(this)->OnNextPage(context);
  }

protected:
  PagedResponse() = default;
  PagedResponse(PagedResponse&&) = default;
  PagedResponse& operator=(PagedResponse&&) = default;
};

} // namespace Azure
