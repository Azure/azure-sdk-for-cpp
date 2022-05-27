
#include "azure/core/internal/input_sanitizer.hpp"
#include "azure/core/url.hpp"
#include <regex>
#include <sstream>

namespace Azure { namespace Core { namespace _internal {

  Azure::Core::Url InputSanitizer::SanitizeUrl(Azure::Core::Url const& url) const
  {
    std::ostringstream ss;

    // Sanitize the non-query part of the URL (remove username and password).
    if (!url.GetScheme().empty())
    {
      ss << url.GetScheme() << "://";
    }
    ss << url.GetHost();
    if (url.GetPort() != 0)
    {
      ss << ":" << url.GetPort();
    }
    if (!url.GetPath().empty())
    {
      ss << "/" << url.GetPath();
    }

    {
      auto encodedRequestQueryParams = url.GetQueryParameters();

      std::remove_const<std::remove_reference<decltype(encodedRequestQueryParams)>::type>::type
          loggedQueryParams;

      if (!encodedRequestQueryParams.empty())
      {
        auto const& unencodedAllowedQueryParams = m_allowedHttpQueryParameters;
        if (!unencodedAllowedQueryParams.empty())
        {
          std::remove_const<std::remove_reference<decltype(unencodedAllowedQueryParams)>::type>::
              type encodedAllowedQueryParams;
          std::transform(
              unencodedAllowedQueryParams.begin(),
              unencodedAllowedQueryParams.end(),
              std::inserter(encodedAllowedQueryParams, encodedAllowedQueryParams.begin()),
              [](std::string const& s) { return Url::Encode(s); });

          for (auto const& encodedRequestQueryParam : encodedRequestQueryParams)
          {
            if (encodedRequestQueryParam.second.empty()
                || (encodedAllowedQueryParams.find(encodedRequestQueryParam.first)
                    != encodedAllowedQueryParams.end()))
            {
              loggedQueryParams.insert(encodedRequestQueryParam);
            }
            else
            {
              loggedQueryParams.insert(
                  std::make_pair(encodedRequestQueryParam.first, m_RedactedPlaceholder));
            }
          }
        }
        else
        {
          for (auto const& encodedRequestQueryParam : encodedRequestQueryParams)
          {
            loggedQueryParams.insert(
                std::make_pair(encodedRequestQueryParam.first, m_RedactedPlaceholder));
          }
        }

        ss << Azure::Core::_detail::FormatEncodedUrlQueryParameters(loggedQueryParams);
      }
    }
    return Azure::Core::Url(ss.str());
  }

  std::string InputSanitizer::SanitizeHeader(std::string const& header, std::string const& value)
      const
  {
    if (m_allowedHttpHeaders.find(header) != m_allowedHttpHeaders.end())
    {
      return value;
    }
    return m_RedactedPlaceholder;
  }
}}} // namespace Azure::Core::_internal
