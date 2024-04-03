// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/http/http.hpp>

#include <memory>
#include <mutex>
#include <string>
namespace Azure { namespace Data { namespace Tables { namespace _detail { namespace Policies {
  class SharedKeyLitePolicy;
}}}}} // namespace Azure::Data::Tables::_detail::Policies

namespace Azure { namespace Data { namespace Tables { namespace Sas {
  class AccountSasBuilder;
  class TablesSasBuilder;
}}}} // namespace Azure::Data::Tables::Sas

namespace Azure { namespace Data { namespace Tables { namespace Credentials {

  /**
   * @brief A NamedKeyCredential is a credential backed by an account's name and
   * one of its access keys.
   */
  class NamedKeyCredential final {
  public:
    /**
     * @brief Initializes a new instance of the NamedKeyCredential.
     *
     * @param accountName Name of the  account.
     * @param accountKey Access key of the
     * account.
     */
    explicit NamedKeyCredential(std::string accountName, std::string accountKey)
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
    friend class Azure::Data::Tables::_detail::Policies::SharedKeyLitePolicy;
    friend class Azure::Data::Tables::Sas::AccountSasBuilder;
    friend class Azure::Data::Tables::Sas::TablesSasBuilder;

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
      std::shared_ptr<NamedKeyCredential> KeyCredential;
    };

    ConnectionStringParts ParseConnectionString(const std::string& connectionString);
    std::string GetDefaultScopeForAudience(const std::string& audience);
  } // namespace _detail

}}}} // namespace Azure::Data::Tables::Credentials
