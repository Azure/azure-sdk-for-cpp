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
  std::string CurrentPageToken;
  std::string NextPageToken;
  std::unique_ptr<Azure::Core::Http::RawResponse> RawResponse;

  bool HasMorePages() const { return m_hasMorePages; }

  void MoveToNextPage(const Azure::Core::Context& context = Azure::Core::Context())
  {
    static_assert(
        std::is_base_of<PagedResponse, Derived>::value,
        "The template argument \"Derived\" should derive from PagedResponse<Derived>.");

    if (NextPageToken.empty())
    {
      m_hasMorePages = false;
      return;
    }
    // Developer of Derived class should make sure current page is kept unchanged if OnNextPage()
    // throws exception.
    static_cast<Derived*>(this)->OnNextPage(context);
  }

protected:
  PagedResponse() = default;
  PagedResponse(PagedResponse&&) = default;
  PagedResponse& operator=(PagedResponse&&) = default;

private:
  bool m_hasMorePages = true;
};

} // namespace Azure
