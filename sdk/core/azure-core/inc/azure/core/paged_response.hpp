// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provides support for responses of paginated collections from the service.
 */

#pragma once

#include <cstdlib>
#include <string>

#include "azure/core/context.hpp"
#include "azure/core/http/raw_response.hpp"
#include "azure/core/nullable.hpp"

namespace Azure { namespace Core {

  /**
   * @brief Defines the base type and behavior for a paged response.
   *
   * @remark The template is used for static-inheritance.
   *
   * @remark T classes must implement the way to get and move to the next page.
   *
   * @tparam T A class type for static-inheritance.
   */
  template <class T> class PagedResponse {
  private:
    // The field used to check when the end of the response is reached. We default it true as the
    // starting point because all responses from a service will always come with a payload that
    // represents at least one page. The page might or might not contain elements in the page.
    // `m_hasPage` is then turned to `false` once `MoveToNextPage` is called on the last page.
    bool m_hasPage = true;

  protected:
    PagedResponse() = default;
    PagedResponse(PagedResponse&&) = default;
    PagedResponse& operator=(PagedResponse&&) = default;

  public:
    virtual ~PagedResponse() = default;

    /**
     * @brief Defines the token used to fetch the current page.
     *
     */
    std::string CurrentPageToken;

    /**
     * @brief Defines the token for getting the next page.
     *
     * @remark If there are no more pages, this field becomes an empty string.
     *
     * @remark Assumes all services will include NextPageToken in the payload, it is set to either
     * null or empty for the last page or to a value used for getting the next page.
     *
     */
    Azure::Nullable<std::string> NextPageToken;

    /**
     * @brief The HTTP response returned by the service.
     *
     */
    std::unique_ptr<Azure::Core::Http::RawResponse> RawResponse;

    /**
     * @brief Check if a page exists. It returns false after the last page.
     *
     */
    bool HasPage() const { return m_hasPage; }

    /**
     * @brief Move to the next page of the response.
     *
     * @remark Calling this method on the last page will set #HasPage() to false.
     *
     * @param context An #Azure::Core::Context controlling the request lifetime.
     */
    void MoveToNextPage(const Azure::Core::Context& context = Azure::Core::Context())
    {
      static_assert(
          std::is_base_of<PagedResponse, T>::value,
          "The template argument \"T\" should derive from PagedResponse<T>.");

      if (!NextPageToken.HasValue() || NextPageToken.Value().empty())
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
