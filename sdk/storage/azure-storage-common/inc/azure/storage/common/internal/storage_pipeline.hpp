// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../storage_credential.hpp"

#include <azure/core/internal/http/pipeline.hpp>

namespace Azure { namespace Storage { namespace _internal {

  std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> ConstructStorageHttpPipeline(
      const std::string& apiVersion,
      const std::string& telemetryPackageName,
      const std::string& telemetryPackageVersion,
      std::shared_ptr<StorageSharedKeyCredential> sharedKeyCredential,
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perCallPolicies,
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies,
      const Azure::Core::_internal::ClientOptions& clientOptions);

}}} // namespace Azure::Storage::_internal