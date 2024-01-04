// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/http/http.hpp>

#include <memory>
#include <mutex>
#include <string>
namespace Azure { namespace Core { namespace Http { namespace Policies { namespace _internal {
  class SharedKeyPolicy;
  class SharedKeyLitePolicy;
}}}}} // namespace Azure::Core::Http::Policies::_internal
namespace Azure { namespace Core { namespace Credentials {

  namespace Sas {
    struct AccountSasBuilder;
  } // namespace Sas

  /**
   * @brief A SharedKeyCredential is a credential backed by an account's name and
   * one of its access keys.
   */
  class SharedKeyCredential final {
  public:
    /**
     * @brief Initializes a new instance of the SharedKeyCredential.
     *
     * @param accountName Name of the  account.
     * @param accountKey Access key of the 
     * account.
     */
    explicit SharedKeyCredential(std::string accountName, std::string accountKey)
        : AccountName(std::move(accountName)), m_accountKey(std::move(accountKey))
    {
    }

    /**
     * @brief Update the  account's access key. This intended to be used when you've
     * regenerated your account's access keys and want to update long lived clients.
     *
     * @param accountKey An  account access key.
     */
    void Update(std::string accountKey)
    {
      std::lock_guard<std::mutex> guard(m_mutex);
      m_accountKey = std::move(accountKey);
    }

    /**
     * @brief Gets the name of the Account.
     */
    const std::string AccountName;

  private:
    friend class Azure::Core::Http::Policies::_internal::SharedKeyPolicy;
    friend class Azure::Core::Http::Policies::_internal::SharedKeyLitePolicy;
    friend struct Sas::AccountSasBuilder;
    std::string GetAccountKey() const
    {
      std::lock_guard<std::mutex> guard(m_mutex);
      return m_accountKey;
    }

    mutable std::mutex m_mutex;
    std::string m_accountKey;
  };

  namespace _internal {

    struct ConnectionStringParts
    {
      std::string AccountName;
      std::string AccountKey;
      Azure::Core::Url BlobServiceUrl;
      Azure::Core::Url FileServiceUrl;
      Azure::Core::Url QueueServiceUrl;
      Azure::Core::Url DataLakeServiceUrl;
      Azure::Core::Url TableServiceUrl;
      std::shared_ptr<SharedKeyCredential> KeyCredential;
    };

    ConnectionStringParts ParseConnectionString(const std::string& connectionString);

    std::string GetDefaultScopeForAudience(const std::string& audience);

  } // namespace _internal

}}} // namespace Azure::Core::Credentials
