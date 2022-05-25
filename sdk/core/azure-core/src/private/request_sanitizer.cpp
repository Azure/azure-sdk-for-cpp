
#include "azure/core/internal/request_sanitizer.hpp"
#include "azure/core/url.hpp"
#include <regex>

namespace Azure { namespace Core { namespace _internal {

  Azure::Core::Url InputSanitizer::SanitizeUrl(Azure::Core::Url const& url)
  {
    Azure::Core::Url redactedUrl;
    redactedUrl.SetScheme(url.GetScheme());
    auto host = url.GetHost();
    auto hostWithNoAccount = std::find(host.begin(), host.end(), '.');
    redactedUrl.SetHost("REDACTED" + std::string(hostWithNoAccount, host.end()));
    // replace any uniqueID from the path for a hardcoded id
    // For the regex, we should not assume anything about the version of UUID format being used. So,
    // using the most general regex to get any uuid version.
    redactedUrl.SetPath(std::regex_replace(
        url.GetPath(),
        std::regex("[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}"),
        "33333333-3333-3333-3333-333333333333"));
    // Query parameters
    for (auto const& qp : url.GetQueryParameters())
    {
      if (qp.first == "sig")
      {
        redactedUrl.AppendQueryParameter("sig", "REDACTED");
      }
      else
      {
        redactedUrl.AppendQueryParameter(qp.first, qp.second);
      }
    }
    return redactedUrl;
  }
}}} // namespace Azure::Core::_internal
