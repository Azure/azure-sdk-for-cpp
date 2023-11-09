// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/identity/chained_token_credential.hpp"

#if defined(TESTING_BUILD)
class DefaultAzureCredential_CachingCredential_Test;
#endif

namespace Azure { namespace Identity { namespace _detail {

  class ChainedTokenCredentialImpl final {

#if defined(TESTING_BUILD)
    // make tests classes friends to validate caching
    friend class DefaultAzureCredential_CachingCredential_Test;
#endif

  public:
    ChainedTokenCredentialImpl(
        std::string const& credentialName,
        ChainedTokenCredential::Sources&& sources,
        bool cacheSelectedCredential = false);

    Core::Credentials::AccessToken GetToken(
        std::string const& credentialName,
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context);

  private:
    std::vector<std::shared_ptr<Core::Credentials::TokenCredential>> m_sources;
    bool m_cacheSelectedCredential;
    // This needs to be atomic so that sentinel comparison is thread safe.
    std::atomic<std::size_t> m_SelectedCredentialIndex = std::numeric_limits<std::size_t>::max();
    std::mutex m_cachingMutex;
  };

}}} // namespace Azure::Identity::_detail
