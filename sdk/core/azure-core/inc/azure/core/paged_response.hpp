// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provides support for paged responses.
 */

#pragma once

#include <cstdlib>
#include <string>

#include "azure/core/context.hpp"
#include "azure/core/http/raw_response.hpp"

namespace Azure { namespace Core {

  /**
   * @brief Defines the base type and behavior for a paged response.
   *
   */
  class PagedResponse {
  private:
    // Derived classes must implement the way to get and move to the next page.
    virtual void OnNextPage(const Azure::Core::Context& context) = 0;
    bool m_endOfResponse = false;

  protected:
    PagedResponse() = default;
    PagedResponse(PagedResponse&&) = default;
    PagedResponse& operator=(PagedResponse&&) = default;

  public:
    /**
     * @brief Defines the token used to fetch the current page.
     *
     */
    std::string CurrentPageToken;

    /**
     * @brief Defines the token for getting a next page.
     *
     * @remark If there is not a next page, this field becomes an empty string.
     *
     * @remark Assumes all services will include NextPageToken in the payload, either null or empty
     * for last page or a value for getting the next page.
     *
     */
    std::string NextPageToken;

    /**
     * @brief The HTTP response returned by the service.
     *
     */
    std::unique_ptr<Azure::Core::Http::RawResponse> RawResponse;

    /**
     * @brief Check if the page has gone after the last page to the end sentinel.
     *
     */
    bool IsEndOfResponse() const { return m_endOfResponse; }

    /**
     * @brief If the page is not at the end of the response and it contains a next
     * page token.
     *
     */
    bool HasMorePages() const { return !m_endOfResponse && !NextPageToken.empty(); }

    /**
     * @brief Get the next page.
     *
     * @param context A #Azure::Core::Context controlling the request lifetime.
     */
    void MoveToNextPage(const Azure::Core::Context& context = Azure::Core::Context())
    {
      if (!HasMorePages())
      {
        m_endOfResponse = true;
        return;
      }
      // Developer should make sure current page is kept unchanged if OnNextPage()
      // throws exception.
      OnNextPage(context);
    }
  };

}} // namespace Azure::Core
