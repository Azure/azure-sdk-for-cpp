// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Websockets functionality.
 */

#pragma once

#include "azure/core/dll_import_export.hpp"
#include "azure/core/exception.hpp"
#include "azure/core/io/body_stream.hpp"
#include "azure/core/url.hpp"

#include <functional>
#include <memory>
#include <string>

namespace Azure { namespace Core { namespace Websockets {

  /**
   * @brief Configuration for a websocket client.
   *
   */
  struct WebsocketClientOptions
  {
  };

  /**
   * @brief The different types of websocket message.
   *
   * @details Text type contains UTF-8 encoded data. Interpretation of Binary type is left to the
   * application.
   *
   */
  enum class WebsocketMessageType
  {
    TextMessage,
    BinaryMessage,
    Close,
    Ping,
    Pong
  };

  /**
   * @brief Template class for a websocket message.
   *
   */
  template <class T> class WebsocketMessage {
  protected:
    Azure::Core::IO::BodyStream& m_bufferStream;

    /**
     * @brief Destroy the Websocket Message object
     *
     */
    ~WebsocketMessage() = default;

  public:
    /**
     * @brief Construct a new Websocket Message object
     *
     * @param type
     * @param bufferStream
     */
    explicit WebsocketMessage(WebsocketMessageType type, Azure::Core::IO::BodyStream& bufferStream)
        : m_bufferStream(bufferStream), Type(type)
    {
    }

    /**
     * @brief Type of message.
     *
     */
    WebsocketMessageType Type;
  };

  /**
   * @brief A websocket message to be sent to a server.
   *
   */
  class WebsocketOutMessage : public WebsocketMessage<WebsocketOutMessage> {
  public:
    explicit WebsocketOutMessage(
        WebsocketMessageType type,
        Azure::Core::IO::BodyStream& bufferStream)
        : WebsocketMessage(type, bufferStream)
    {
    }
  };

  /**
   * @brief A websocket message received by the server.
   *
   */
  class WebsocketInMessage : public WebsocketMessage<WebsocketInMessage> {
  public:
    explicit WebsocketInMessage(
        WebsocketMessageType type,
        Azure::Core::IO::BodyStream& bufferStream)
        : WebsocketMessage(type, bufferStream)
    {
    }
  };

  /**
   * @brief Abstract class which defines the behavior for a websocket client.
   *
   */
  class WebsocketClientImplementation {
  public:
    /**
     * @brief Base constructor for all websocket client implementations.
     *
     * @param url A url to the websocket server.
     * @param clientOptions The configuration for the websocket implementation.
     */
    explicit WebsocketClientImplementation(
        Azure::Core::Url url,
        WebsocketClientOptions clientOptions)
        : m_url(std::move(url)), m_clientOptions(std::move(clientOptions))
    {
    }

    /**
     * @brief Destroy the Websocket Client Implementation object
     *
     */
    virtual ~WebsocketClientImplementation() {}

    /**
     * @brief Stablish network connection to the websocket server.
     *
     */
    virtual void Connect() = 0;

    /**
     * @brief Request connection to be closed.
     *
     */
    virtual void Close() = 0;

    /**
     * @brief Send a message to the websocket server.
     *
     * @param message The message to be sent to the server.
     * @param context A context to control the request lifetime.
     */
    virtual void Send(WebsocketOutMessage& message, Azure::Core::Context const& context) = 0;

    /**
     * @brief Set a callback to be called when a message is received from the server.
     *
     * @param handler A callback function which gets the incoming message from network.
     */
    virtual void OnMessage(std::function<void(WebsocketInMessage const&)> const& handler) = 0;

  protected:
    Azure::Core::Url m_url;
    WebsocketClientOptions m_clientOptions;
  };

  /**
   * @brief Websocket client class provides network communication with a server, using websocket
   * protoccol.
   *
   */
  class WebsocketClient {
  private:
    std::unique_ptr<WebsocketClientImplementation> m_client;

  public:
    /**
     * @brief Construct a new Websocket Client object.
     *
     * @param clientOptions Optional configuration used to create the websocket client.
     */
    explicit WebsocketClient(
        Azure::Core::Url url,
        WebsocketClientOptions clientOptions = WebsocketClientOptions{});

    /**
     * @brief Stablish network connection to the websocket server.
     *
     */
    void Connect() { m_client->Connect(); }

    /**
     * @brief Request connection to be closed.
     *
     */
    void Close() { m_client->Close(); }

    /**
     * @brief Send a message to the websocket server.
     *
     * @param message The message to be sent to the server.
     * @param context A context to control the request lifetime.
     */
    void Send(WebsocketOutMessage& message, Azure::Core::Context const& context)
    {
      m_client->Send(message, context);
    }

    /**
     * @brief Set a callback to be called when a message is received from the server.
     *
     * @param handler A callback function which gets the incoming message from network.
     */
    void OnMessage(std::function<void(WebsocketInMessage const&)> const& handler)
    {
      m_client->OnMessage(handler);
    }
  };

}}} // namespace Azure::Core::Websockets
