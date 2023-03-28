// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "network/sasl_transport.hpp"

#include <chrono>
#include <memory>
#include <string>

namespace Azure { namespace Core { namespace _internal { namespace Amqp {
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

    virtual std::shared_ptr<Network::Transport> GetTransport() const = 0;
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

  class ServiceBusSasConnectionStringCredential final : public ConnectionStringCredential {
  public:
    ServiceBusSasConnectionStringCredential(
        const std::string& connectionString,
        const std::string& entityPath = {})
        : ConnectionStringCredential(connectionString, CredentialType::ServiceBusSas)
    {
      if (m_entityPath.empty())
      {
        m_entityPath = entityPath;
      }
      else if (!entityPath.empty() && m_entityPath != entityPath)
      {
        throw std::invalid_argument("Unable to determine entityPath.");
      }
    }

    ~ServiceBusSasConnectionStringCredential() override = default;
    std::string GenerateSasToken(std::chrono::system_clock::time_point const& expiresOn) const;
    std::string GetAudience();

    // Return a SASL transport configured for SASL Anonymous.
    virtual std::shared_ptr<Network::Transport> GetTransport() const override;
  };

  class SaslPlainConnectionStringCredential final : public ConnectionStringCredential {
  public:
    SaslPlainConnectionStringCredential(const std::string& connectionString);

    std::shared_ptr<Network::Transport> GetTransport() const override;

  private:
  };
}}}} // namespace Azure::Core::_internal::Amqp
