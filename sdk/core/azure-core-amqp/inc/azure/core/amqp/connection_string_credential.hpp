// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "network/sasl_transport.hpp"
#include <chrono>
#include <memory>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  //
  // A ServiceBus connection string has the following format:
  // "Endpoint=sb://<namespace>.servicebus.windows.net/;SharedAccessKeyName=<KeyName>;SharedAccessKey=<KeyValue>;EntityPath=<entity>"
  //

  enum class CredentialType
  {
    None,
    SaslPlain,
    ServiceBusSas,
    BearerToken,
  };

  class ConnectionStringCredential {
  public:
    ConnectionStringCredential(const std::string& connectionString, CredentialType credentialType)
        : m_credentialType{credentialType}
    {
      ParseConnectionString(connectionString);
    }
    virtual ~ConnectionStringCredential() = default;

    // Prevent tearing of the ConnectionStringCredential.
    ConnectionStringCredential(ConnectionStringCredential const&) = delete;
    ConnectionStringCredential& operator=(ConnectionStringCredential const&) = delete;

    virtual std::shared_ptr<Network::_internal::Transport> GetTransport() const = 0;
    CredentialType GetCredentialType() const { return m_credentialType; }
    std::string const& GetEndpoint() const { return m_endpoint; }
    std::string const& GetSharedAccessKeyName() const { return m_sharedAccessKeyName; }
    std::string const& GetSharedAccessKey() const { return m_sharedAccessKey; }
    std::string const& GetEntityPath() const { return m_entityPath; }
    std::string const& GetHostName() const { return m_hostName; }
    uint16_t GetPort() const { return m_port; }

  private:
    void ParseConnectionString(const std::string& connectionString);
    const CredentialType m_credentialType;
    std::string m_endpoint;
    std::string m_sharedAccessKeyName;
    std::string m_sharedAccessKey;
    std::string m_uri;
    std::string m_hostName;
    uint16_t m_port;

  protected:
    std::string m_entityPath;
  };

  /** @brief A connection string based credential used for AMQP Connection Based Security using an
   * SAS token.
   */
  class ServiceBusSasConnectionStringCredential final : public ConnectionStringCredential {
  public:
    /** @brief Create an instance of the ServiceBusSasConnectionStringCredential.
     *
     * @param connectionString The connection string for the Service Bus namespace.
     * @param entityPath The name of the entity to connect to.
     *
     * @remark If the connectionString contains an EntityPath element, and the entityPath parameter
     * is provided, this constructor will throw an exception if the two values do not match.
     *
     */
    ServiceBusSasConnectionStringCredential(
        const std::string& connectionString,
        const std::string& entityPath = {})
        : ConnectionStringCredential(connectionString, CredentialType::ServiceBusSas)
    {
      // If we weren't able to determine the entity path from the ConnectionStringCredential
      // constructor, use the entity path passed in by the user.
      if (m_entityPath.empty())
      {
        m_entityPath = entityPath;
      }
      else if (!entityPath.empty() && m_entityPath != entityPath)
      {
        // If the user provided an entity path, but it doesn't match the one in the connection
        // string, throw.
        throw std::invalid_argument("Unable to determine entityPath.");
      }
    }

    /** @brief Destroy a SAS connection string credential. */
    ~ServiceBusSasConnectionStringCredential() override = default;

    /** @brief Generate an SAS token with the specified expiration time for this connection string
     * credential.
     *
     * @param expiresOn The expiration time for the SAS token.
     */
    std::string GenerateSasToken(std::chrono::system_clock::time_point const& expiresOn) const;

    /** @brief Returns the expected audience for this credential.
     */
    std::string GetAudience();

    /** @brief Return a SASL transport configured for SASL Anonymous which will be used to
     * communicate with the AMQP service.
     *
     * @return A SASL transport configured for SASL Anonymous.
     */
    virtual std::shared_ptr<Network::_internal::Transport> GetTransport() const override;
  };

  /** A SASL PLAIN connection string credential.
   *
   * @note This credential type is not supported for Service Bus.
   *
   */
  class SaslPlainConnectionStringCredential final : public ConnectionStringCredential {
  public:
    SaslPlainConnectionStringCredential(const std::string& connectionString);

    std::shared_ptr<Network::_internal::Transport> GetTransport() const override;

  private:
  };
}}}} // namespace Azure::Core::Amqp::_internal
