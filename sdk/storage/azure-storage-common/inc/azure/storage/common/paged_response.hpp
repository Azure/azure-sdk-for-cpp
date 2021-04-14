// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

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
    if (!HasMore())
    {
      return;
    }
    CurrentPageToken = NextPageToken;
    static_cast<Derived*>(this)->OnNextPage(context);
  }

protected:
  explicit PagedResponse(std::string currentPageToken)
      : CurrentPageToken(std::move(currentPageToken))
  {
    static_assert(
        std::is_base_of<PagedResponse, Derived>::value,
        "The template argument \"Derived\" should derive from PagedResponse<Derived>.");
  }
  PagedResponse(PagedResponse&&) = default;
  PagedResponse& operator=(PagedResponse&&) = default;
};

} // namespace Azure
