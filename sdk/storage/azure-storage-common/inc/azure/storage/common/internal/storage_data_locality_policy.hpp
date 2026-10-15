// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/storage/common/dll_import_export.hpp"

#include <azure/core/http/policies/policy.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace _internal {

  AZ_STORAGE_COMMON_DLLEXPORT extern const Azure::Core::Context::Key DataLocalityEndpointKey;

  inline Azure::Core::Context WithDataLocalityEndpoint(
      const Azure::Core::Context& context,
      const std::string& endpoint)
  {
    return endpoint.empty() ? context : context.WithValue(DataLocalityEndpointKey, endpoint);
  }

  class StorageDataLocalityPolicy final : public Azure::Core::Http::Policies::HttpPolicy {
  public:
    std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> Clone() const override
    {
      return std::make_unique<StorageDataLocalityPolicy>(*this);
    }

    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Http::Request& request,
        Azure::Core::Http::Policies::NextHttpPolicy nextPolicy,
        const Azure::Core::Context& context) const override;
  };

}}} // namespace Azure::Storage::_internal
