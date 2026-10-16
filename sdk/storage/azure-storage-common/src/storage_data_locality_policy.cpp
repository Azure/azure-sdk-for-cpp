// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/storage/common/internal/storage_data_locality_policy.hpp>

namespace Azure { namespace Storage { namespace _internal {

  const Azure::Core::Context::Key DataLocalityEndpointKey;

  std::unique_ptr<Azure::Core::Http::RawResponse> StorageDataLocalityPolicy::Send(
      Azure::Core::Http::Request& request,
      Azure::Core::Http::Policies::NextHttpPolicy nextPolicy,
      const Azure::Core::Context& context) const
  {
    std::string endpoint;
    if (!context.TryGetValue(DataLocalityEndpointKey, endpoint) || endpoint.empty())
    {
      return nextPolicy.Send(request, context);
    }

    auto& requestUrl = request.GetUrl();
    std::string originalAuthority = requestUrl.GetHost();
    if (requestUrl.GetPort() != 0)
    {
      originalAuthority += ":" + std::to_string(requestUrl.GetPort());
    }

    // The Storage service returns endpoint values as absolute URLs with a scheme and port.
    Azure::Core::Url endpointUrl(
        endpoint.find("://") == std::string::npos ? requestUrl.GetScheme() + "://" + endpoint
                                                  : endpoint);
    request.SetHeader("Host", originalAuthority);
    requestUrl.SetScheme(endpointUrl.GetScheme());
    requestUrl.SetHost(endpointUrl.GetHost());
    requestUrl.SetPort(endpointUrl.GetPort());
    return nextPolicy.Send(request, context);
  }

}}} // namespace Azure::Storage::_internal
