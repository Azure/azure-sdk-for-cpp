// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/http/http.hpp>

#include <memory>
#include <mutex>
#include <string>
namespace Azure { namespace Data { namespace Tables { namespace _detail { namespace Policies {
  class SharedKeyPolicy;
  class SharedKeyLitePolicy;
}}}}} // namespace Azure::Data::Tables::_detail::Policies

namespace Azure { namespace Data { namespace Tables { namespace Sas {
  struct AccountSasBuilder;
}}}} // namespace Azure::Data::Tables::Sas

namespace Azure { namespace Data { namespace Tables { namespace Credentials {

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
        : AccountName{std::move(accountName)}, m_accountKey{std::move(accountKey)}
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
     * @brief The name of the Account.
     */
    const std::string AccountName;

  private:
    friend class Azure::Data::Tables::_detail::Policies::SharedKeyPolicy;
    friend class Azure::Data::Tables::_detail::Policies::SharedKeyLitePolicy;
    friend struct Azure::Data::Tables::Sas::AccountSasBuilder;

    std::string GetAccountKey() const
    {
      std::lock_guard<std::mutex> guard(m_mutex);
      return m_accountKey;
    }

    mutable std::mutex m_mutex;
    std::string m_accountKey;
  };

  namespace _detail {
    struct ConnectionStringParts
    {
      std::string AccountName;
      std::string AccountKey;
      Azure::Core::Url TableServiceUrl;
      std::shared_ptr<SharedKeyCredential> KeyCredential;
    };

    ConnectionStringParts ParseConnectionString(const std::string& connectionString);
    std::string GetDefaultScopeForAudience(const std::string& audience);
  } // namespace _detail

}}}} // namespace Azure::Data::Tables::Credentials
