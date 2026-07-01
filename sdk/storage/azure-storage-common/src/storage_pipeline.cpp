// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/common/internal/storage_pipeline.hpp"

#include "azure/storage/common/internal/constants.hpp"
#include "azure/storage/common/internal/storage_per_retry_policy.hpp"
#include "azure/storage/common/internal/storage_retry_policy.hpp"
#include "azure/storage/common/internal/storage_service_version_policy.hpp"
#include "azure/storage/common/internal/storage_switch_to_secondary_policy.hpp"

#include <azure/core/internal/http/http_sanitizer.hpp>

namespace Azure { namespace Storage { namespace _internal {

  std::vector<std::unique_ptr<Core::Http::Policies::HttpPolicy>> BuildHttpPipelinePolicies(
      const Core::_internal::ClientOptions& clientOptions,
      BuildStoragePipelineOptions storagePipelineOptions)
  {
    using Core::Http::Policies::HttpPolicy;

    std::vector<std::unique_ptr<HttpPolicy>> policies;

    // service per call policies
    if (!storagePipelineOptions.ApiVersion.empty())
    {
      policies.emplace_back(
          std::make_unique<StorageServiceVersionPolicy>(storagePipelineOptions.ApiVersion));
    }
    policies.emplace_back(std::make_unique<Core::Http::Policies::_internal::RequestIdPolicy>());
    if (!storagePipelineOptions.PackageName.empty())
    {
      policies.emplace_back(std::make_unique<Core::Http::Policies::_internal::TelemetryPolicy>(
          storagePipelineOptions.PackageName,
          storagePipelineOptions.PackageVersion,
          clientOptions.Telemetry));
    }

    // client-options per call policies
    for (auto& policy : clientOptions.PerOperationPolicies)
    {
      policies.emplace_back(policy->Clone());
    }

    // Retry policy
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(clientOptions.Retry));

    // service per-retry: switch-to-secondary, storage per-retry
    if (!storagePipelineOptions.SecondaryHost.empty())
    {
      policies.emplace_back(std::make_unique<StorageSwitchToSecondaryPolicy>(
          storagePipelineOptions.PrimaryHost, storagePipelineOptions.SecondaryHost));
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());

    // Token/bearer auth runs before user-injected per-retry policies so it sees the
    // original request URL (e.g. before the test-proxy URL-rewrite policy).
    if (storagePipelineOptions.TokenAuthPolicy)
    {
      policies.emplace_back(std::move(storagePipelineOptions.TokenAuthPolicy));
    }

    // client options per-retry policies
    for (auto& policy : clientOptions.PerRetryPolicies)
    {
      policies.emplace_back(policy->Clone());
    }

    // SharedKey/SAS auth runs last among per-retry policies so the signature covers
    // the final, fully-modified request.
    if (storagePipelineOptions.SharedKeyAuthPolicy)
    {
      policies.emplace_back(std::move(storagePipelineOptions.SharedKeyAuthPolicy));
    }

    // Request activity policy (distributed tracing)
    Core::Http::_internal::HttpSanitizer httpSanitizer(
        clientOptions.Log.AllowedHttpQueryParameters, clientOptions.Log.AllowedHttpHeaders);
    policies.emplace_back(
        std::make_unique<Core::Http::Policies::_internal::RequestActivityPolicy>(httpSanitizer));

    // Logging - won't update request
    policies.emplace_back(
        std::make_unique<Core::Http::Policies::_internal::LogPolicy>(clientOptions.Log));

    // Transport
    policies.emplace_back(std::make_unique<Core::Http::Policies::_internal::TransportPolicy>(
        clientOptions.Transport));

    return policies;
  }

}}} // namespace Azure::Storage::_internal
