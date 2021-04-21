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
   * @remark The template is using for making static-inheritance.
   *
   * @remark T classes must implement the way to get and move to the next page.
   *
   * @tparam T A class type for static-inheritance.
   */
  template <class T> class PagedResponse {
  private:
    // The field used to check when the end of the response is reached. All responses from a service
    // will always come with a payload that represents a page. The page might or might not contain
    // elements in the page. `m_hasPage` is then turned to `false` once `MoveToNextPage` is called
    // on the last page.
    bool m_hasPage = true;

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
    bool HasPage() const { return m_hasPage; }

    /**
     * @brief Get the next page.
     *
     * @remark Calling this method on the last page will turned #HasPage() to false.
     *
     * @param context A #Azure::Core::Context controlling the request lifetime.
     */
    void MoveToNextPage(const Azure::Core::Context& context = Azure::Core::Context())
    {
      static_assert(
          std::is_base_of<PagedResponse, T>::value,
          "The template argument \"T\" should derive from PagedResponse<T>.");

      if (NextPageToken.empty())
      {
        m_hasPage = false;
        return;
      }

      // Developer must make sure current page is kept unchanged if OnNextPage()
      // throws exception.
      static_cast<T*>(this)->OnNextPage(context);
    }
  };

}} // namespace Azure::Core
