// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "network/sasl_transport.hpp"

#include <azure/core/credentials/credentials.hpp>

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  //
  // A ServiceBus connection string has the following format:
  // "Endpoint=sb://<namespace>.servicebus.windows.net/;SharedAccessKeyName=<KeyName>;SharedAccessKey=<KeyValue>;EntityPath=<entity>"
  //

  class ConnectionStringParser final {
  public:
    ConnectionStringParser(const std::string& connectionString)
    {
      ParseConnectionString(connectionString);
    }
    ConnectionStringParser(const ConnectionStringParser&) = default;
    ConnectionStringParser& operator=(const ConnectionStringParser&) = default;
    ConnectionStringParser(ConnectionStringParser&&) = default;
    ConnectionStringParser& operator=(ConnectionStringParser&&) = default;
    ~ConnectionStringParser() = default;

    std::string const& GetEndpoint() const { return m_endpoint; }
    std::string const& GetSharedAccessKeyName() const { return m_sharedAccessKeyName; }
    std::string const& GetSharedAccessKey() const { return m_sharedAccessKey; }
    std::string const& GetEntityPath() const { return m_entityPath; }
    std::string const& GetHostName() const { return m_hostName; }
    uint16_t GetPort() const { return m_port; }
    bool UseDevelopmentEmulator() const { return m_useDevelopmentEmulator; }

  private:
    void ParseConnectionString(const std::string& connectionString);
    std::string m_endpoint;
    std::string m_sharedAccessKeyName;
    std::string m_sharedAccessKey;
    std::string m_uri;
    std::string m_hostName;
    bool m_useDevelopmentEmulator{false};
    uint16_t m_port{};
    std::string m_entityPath;
  };

  /** @brief A connection string based credential used for AMQP Connection Based Security
   * using a SAS token.
   */
  class ServiceBusSasConnectionStringCredential final : public Credentials::TokenCredential {
  public:
    /** @brief Create an instance of the ServiceBusSasConnectionStringCredential.
     *
     * @param connectionString The connection string for the Service Bus namespace.
     * @param entityPath The name of the entity to connect to.
     *
     * @remark If the connectionString contains an EntityPath element, and the entityPath
     * parameter is provided, this constructor will throw an exception if the two values do
     * not match.
     *
     */
    ServiceBusSasConnectionStringCredential(
        const std::string& connectionString,
        const std::string& entityPath = {})
        : TokenCredential("ServiceBusSasConnectionStringCredential"),
          m_connectionParser(connectionString)
    {
      // If we weren't able to determine the entity path from the ConnectionStringCredential
      // constructor, use the entity path passed in by the user.
      if (m_connectionParser.GetEntityPath().empty())
      {
        m_entityPath = entityPath;
      }
      else if (!entityPath.empty() && m_connectionParser.GetEntityPath() != entityPath)
      {
        // If the user provided an entity path, but it doesn't match the one in the connection
        // string, throw.
        throw std::invalid_argument(
            "Entity Path provided: '" + entityPath
            + "' does not match connection string entity path: '"
            + m_connectionParser.GetEntityPath() + "'.");
      }
    }

    /** @brief Copy constructor
     *
     * @remarks Note that TokenCredential derived objects are expected to be passed via
     * std::shared_ptr and thus should never be directly constructed.
     */
    ServiceBusSasConnectionStringCredential(const ServiceBusSasConnectionStringCredential& other)
        = delete;

    /** @brief Copy assignment operator
     *
     *       * @remarks Note that TokenCredential derived objects are expected to be passed via
     std::shared_ptr and thus
     * should never be directly constructed.

    */
    ServiceBusSasConnectionStringCredential& operator=(
        const ServiceBusSasConnectionStringCredential& other)
        = delete;

    /** @brief Move constructor
     *
     * @remarks Note that TokenCredential derived objects are expected to be passed via
     * std::shared_ptr and thus should never be directly constructed.
     */
    ServiceBusSasConnectionStringCredential(
        ServiceBusSasConnectionStringCredential&& other) noexcept
        = delete;

    /** @brief Move assignment operator
     *
     * @remarks Note that TokenCredential derived objects are expected to be passed via
     * std::shared_ptr and thus should never be directly constructed.
     */
    ServiceBusSasConnectionStringCredential& operator=(
        ServiceBusSasConnectionStringCredential&& other) noexcept
        = delete;

    /** @brief Destroy a SAS connection string credential. */
    ~ServiceBusSasConnectionStringCredential() override = default;

    /** @brief Returns the expected audience for this credential.
     */
    std::string GetAudience();

    /** @brief Return a SASL transport configured for SASL Anonymous which will be used to
     * communicate with the AMQP service.
     *
     * @return A SASL transport configured for SASL Anonymous.
     */
    virtual std::shared_ptr<Network::_internal::Transport> GetTransport() const;

    std::string const& GetEndpoint() const { return m_connectionParser.GetEndpoint(); }
    std::string const& GetSharedAccessKeyName() const
    {
      return m_connectionParser.GetSharedAccessKeyName();
    }
    std::string const& GetSharedAccessKey() const
    {
      return m_connectionParser.GetSharedAccessKey();
    }
    std::string const& GetEntityPath() const { return m_connectionParser.GetEntityPath(); }
    std::string const& GetHostName() const { return m_connectionParser.GetHostName(); }
    uint16_t GetPort() const { return m_connectionParser.GetPort(); }

    bool UseDevelopmentEmulator() { return m_connectionParser.UseDevelopmentEmulator(); }

    /**
     * @brief Gets an authentication token.
     *
     * @param tokenRequestContext A context to get the token in.
     * @param context A context to control the request lifetime.
     *
     * @return Authentication token.
     *
     * @throw Credentials::AuthenticationException Authentication error occurred.
     */
    Credentials::AccessToken GetToken(
        Credentials::TokenRequestContext const& tokenRequestContext,
        Context const& context) const override;

  private:
    ConnectionStringParser m_connectionParser;
    std::string m_entityPath;

    ///** @brief Generate an SAS token with the specified expiration time for this connection
    /// string
    // * credential.
    // *
    // * @param expiresOn The expiration time for the SAS token.
    // */
    std::string GenerateSasToken(std::chrono::system_clock::time_point const& expiresOn) const;
  };

}}}} // namespace Azure::Core::Amqp::_internal
