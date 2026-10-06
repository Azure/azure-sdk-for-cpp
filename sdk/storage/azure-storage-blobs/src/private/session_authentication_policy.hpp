// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/storage/blobs/blob_options.hpp>
#include <azure/storage/common/internal/storage_bearer_token_auth.hpp>

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  struct TokenAuthenticationPolicies final
  {
    std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> TokenAuthPolicy;
    std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> FinalAuthPolicy;
  };

  TokenAuthenticationPolicies CreateTokenAuthenticationPolicies(
      const std::string& clientUrl,
      std::shared_ptr<const Azure::Core::Credentials::TokenCredential> credential,
      Azure::Core::Credentials::TokenRequestContext tokenRequestContext,
      bool enableTenantDiscovery,
      const BlobClientOptions& options);

}}}} // namespace Azure::Storage::Blobs::_detail
