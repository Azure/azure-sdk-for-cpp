// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/client_options.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace _internal {

  struct BuildStoragePipelineOptions final
  {
    std::string PackageName;
    std::string PackageVersion;
    std::string PrimaryHost;
    std::string SecondaryHost;
    std::string ApiVersion;
    std::unique_ptr<Core::Http::Policies::HttpPolicy> TokenAuthPolicy;
    std::unique_ptr<Core::Http::Policies::HttpPolicy> SharedKeyAuthPolicy;
  };

  std::vector<std::unique_ptr<Core::Http::Policies::HttpPolicy>> BuildHttpPipelinePolicies(
      const Core::_internal::ClientOptions& clientOptions,
      BuildStoragePipelineOptions storagePipelineOptions);

}}} // namespace Azure::Storage::_internal
