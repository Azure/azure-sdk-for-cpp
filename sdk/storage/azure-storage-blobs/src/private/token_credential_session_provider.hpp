// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/storage/blobs/blob_options.hpp>

#include <map>
#include <mutex>

namespace Azure { namespace Core { namespace Http { namespace _internal {
  class HttpPipeline;
}}}} // namespace Azure::Core::Http::_internal

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  struct SessionTokenInfo final
  {
    std::string Token;
    std::string Key;
    Azure::DateTime ExpiresOn;
  };

  class TokenCredentialSessionProvider final {
  public:
    TokenCredentialSessionProvider(
        const std::string& serviceUrl,
        std::shared_ptr<const Azure::Core::Credentials::TokenCredential> credential,
        const BlobClientOptions& options);

    Azure::Nullable<SessionTokenInfo> GetSession(
        const Azure::Core::Http::Request& request,
        const Azure::Core::Context& context);

    void InvalidateSession(
        const Azure::Core::Http::Request& request,
        const SessionTokenInfo& session);

    bool IsRequestEligible(const Azure::Core::Http::Request& request) const;

  private:
    struct CacheEntry
    {
      std::mutex Mutex;
      Azure::Nullable<SessionTokenInfo> Current;
      Azure::DateTime CooldownUntil;
      Azure::DateTime LastRefreshStarted;
    };

    Azure::Core::Url m_serviceUrl;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
    std::mutex m_cacheMutex;
    std::map<std::string, std::shared_ptr<CacheEntry>> m_cache;

    std::string GetContainerUrl(const Azure::Core::Http::Request& request) const;
    std::shared_ptr<CacheEntry> GetCacheEntry(const std::string& containerUrl);
    SessionTokenInfo CreateSession(
        const std::string& containerUrl,
        const Azure::Core::Context& context);
  };

}}}} // namespace Azure::Storage::Blobs::_detail
