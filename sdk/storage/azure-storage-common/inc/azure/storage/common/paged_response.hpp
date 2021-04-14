// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdlib>
#include <string>

#include <azure/core/context.hpp>
#include <azure/core/http/raw_response.hpp>

namespace Azure {

template <class Derived> class PagedResponse {
  bool m_hasMore = true;

public:
  std::string NextPageToken;
  std::string CurrentPageToken;
  std::unique_ptr<Azure::Core::Http::RawResponse> RawResponse;

  bool HasMore() const { return m_hasMore; }

  void NextPage(const Azure::Core::Context& context)
  {
    static_assert(
        std::is_base_of<PagedResponse, Derived>::value,
        "The template argument \"Derived\" should derive from PagedResponse<Derived>.");

    if (!m_hasMore)
    {
      // User should check HasMore() before calling NextPage().
      std::abort();
    }
    if (m_hasMore && NextPageToken.empty())
    {
      m_hasMore = false;
      return;
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
