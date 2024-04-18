// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/common/internal/storage_pipeline.hpp"

#include "azure/storage/common/internal/shared_key_policy.hpp"
#include "azure/storage/common/internal/storage_per_retry_policy.hpp"
#include "azure/storage/common/internal/storage_retry_policy.hpp"
#include "azure/storage/common/internal/storage_service_version_policy.hpp"

namespace Azure { namespace Storage { namespace _internal {

  std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> ConstructStorageHttpPipeline(
      const std::string& apiVersion,
      const std::string& telemetryPackageName,
      const std::string& telemetryPackageVersion,
      std::shared_ptr<StorageSharedKeyCredential> sharedKeyCredential,
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perCallPolicies,
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies,
      const Azure::Core::_internal::ClientOptions& clientOptions)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;

    // service-specific per call policies
    for (auto& policy : perCallPolicies)
    {
      policies.push_back(std::move(policy));
    }
    policies.push_back(std::make_unique<_internal::StorageServiceVersionPolicy>(apiVersion));

    // Request Id
    policies.push_back(std::make_unique<Azure::Core::Http::Policies::_internal::RequestIdPolicy>());

    // Telemetry (user-agent header)
    policies.push_back(std::make_unique<Azure::Core::Http::Policies::_internal::TelemetryPolicy>(
        telemetryPackageName, telemetryPackageVersion, clientOptions.Telemetry));

    // client-options per call policies.
    for (auto& policy : clientOptions.PerOperationPolicies)
    {
      policies.push_back(policy->Clone());
    }

    // Retry policy
    policies.push_back(std::make_unique<StorageRetryPolicy>(clientOptions.Retry));

    // service-specific per retry policies.
    for (auto& policy : perRetryPolicies)
    {
      policies.push_back(std::move(policy));
    }
    perRetryPolicies.push_back(std::make_unique<StoragePerRetryPolicy>());

    // client options per retry policies.
    for (auto& policy : clientOptions.PerRetryPolicies)
    {
      policies.push_back(policy->Clone());
    }

    if (sharedKeyCredential)
    {
      auto policy = std::make_unique<SharedKeyPolicy>(sharedKeyCredential);
      policies.push_back(policy->Clone());
    }

    // Policies after here cannot modify the request anymore

    // Add a request activity policy which will generate distributed traces for the pipeline.
    Azure::Core::Http::_internal::HttpSanitizer httpSanitizer(
        clientOptions.Log.AllowedHttpQueryParameters, clientOptions.Log.AllowedHttpHeaders);
    policies.push_back(
        std::make_unique<Azure::Core::Http::Policies::_internal::RequestActivityPolicy>(
            httpSanitizer));

    // logging - won't update request
    policies.push_back(
        std::make_unique<Azure::Core::Http::Policies::_internal::LogPolicy>(clientOptions.Log));

    // transport
    policies.push_back(std::make_unique<Azure::Core::Http::Policies::_internal::TransportPolicy>(
        clientOptions.Transport));

    return std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(policies);
  }

}}} // namespace Azure::Storage::_internal
