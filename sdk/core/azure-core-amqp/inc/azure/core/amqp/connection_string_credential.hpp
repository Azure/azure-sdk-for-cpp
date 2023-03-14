// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

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
    ConnectionStringCredential(const std::string& connectionString)
    {
      ParseConnectionString(connectionString);
    }
    virtual ~ConnectionStringCredential() = default;
    virtual std::shared_ptr<Network::Transport> GetTransport() const = 0;
    virtual CredentialType GetCredentialType() const = 0;
    std::string const& GetUri() const;
    std::string const& GetEndpoint() const { return m_endpoint; }
    std::string const& GetSharedAccessKeyName() const { return m_sharedAccessKeyName; }
    std::string const& GetSharedAccessKey() const { return m_sharedAccessKey; }
    std::string const& GetEntityPath() const { return m_entityPath; }
    std::string const& GetHostName() const { return m_hostName; }
    uint16_t const GetPort() const { return m_port; }

  private:
    void ParseConnectionString(const std::string& connectionString);
    std::string m_endpoint;
    std::string m_sharedAccessKeyName;
    std::string m_sharedAccessKey;
    std::string m_entityPath;
    std::string m_uri;
    std::string m_hostName;
    uint16_t m_port;
  };

  class ServiceBusSasConnectionStringCredential : public ConnectionStringCredential {
  public:
    ServiceBusSasConnectionStringCredential(const std::string& connectionString)
        : ConnectionStringCredential(connectionString)
    {
    }

    ~ServiceBusSasConnectionStringCredential() = default;
    CredentialType GetCredentialType() const override { return CredentialType::ServiceBusSas; }
    std::string GenerateSasToken(std::chrono::system_clock::time_point const& expiresOn) const;
    std::string GetAudience();

    // Return a SASL transport configured for SASL Anonymous.
    virtual std::shared_ptr<Network::Transport> GetTransport() const;
  };

  class SaslPlainConnectionStringCredential : public ConnectionStringCredential {
  public:
    SaslPlainConnectionStringCredential(const std::string& connectionString);

    std::shared_ptr<Network::Transport> GetTransport() const;
    CredentialType GetCredentialType() const override { return CredentialType::SaslPlain; }

  private:
  };
}}}} // namespace Azure::Core::_internal::Amqp
