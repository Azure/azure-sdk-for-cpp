// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief WebSockets functionality.
 */

#pragma once

#include "azure/core/dll_import_export.hpp"
#include "azure/core/exception.hpp"
#include "azure/core/io/body_stream.hpp"
#include "azure/core/url.hpp"

#include <functional>
#include <memory>
#include <string>

namespace Azure { namespace Core { namespace WebSockets {

  /**
   * @brief Configuration for a websocket client.
   *
   */
  struct WebSocketClientOptions
  {
  };

  /**
   * @brief The different types of websocket message.
   *
   * @details Text type contains UTF-8 encoded data. Interpretation of Binary type is left to the
   * application.
   *
   */
  enum class WebSocketMessageType
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
  template <class T> class WebSocketMessage {
  protected:
    Azure::Core::IO::BodyStream& m_bufferStream;

    /**
     * @brief Destroy the WebSocket Message object
     *
     */
    ~WebSocketMessage() = default;

  public:
    /**
     * @brief Construct a new WebSocket Message object
     *
     * @param type
     * @param bufferStream
     */
    explicit WebSocketMessage(WebSocketMessageType type, Azure::Core::IO::BodyStream& bufferStream)
        : m_bufferStream(bufferStream), Type(type)
    {
    }

    /**
     * @brief Type of message.
     *
     */
    WebSocketMessageType Type;
  };

  /**
   * @brief A websocket message to be sent to a server.
   *
   */
  class WebSocketOutMessage : public WebSocketMessage<WebSocketOutMessage> {
  public:
    explicit WebSocketOutMessage(
        WebSocketMessageType type,
        Azure::Core::IO::BodyStream& bufferStream)
        : WebSocketMessage(type, bufferStream)
    {
    }
  };

  /**
   * @brief A websocket message received by the server.
   *
   */
  class WebSocketInMessage : public WebSocketMessage<WebSocketInMessage> {
  public:
    explicit WebSocketInMessage(
        WebSocketMessageType type,
        Azure::Core::IO::BodyStream& bufferStream)
        : WebSocketMessage(type, bufferStream)
    {
    }
  };

  namespace _detail {
    /**
     * @brief Abstract class which defines the behavior for a websocket client.
     *
     */
    class WebSocketClientImplementation {
    public:
      /**
       * @brief Base constructor for all websocket client implementations.
       *
       * @param url A url to the websocket server.
       * @param clientOptions The configuration for the websocket implementation.
       */
      explicit WebSocketClientImplementation(
          Azure::Core::Url url,
          WebSocketClientOptions clientOptions)
          : m_url(std::move(url)), m_clientOptions(std::move(clientOptions))
      {
      }

      /**
       * @brief Destroy the WebSocket Client Implementation object
       *
       */
      virtual ~WebSocketClientImplementation() {}

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
      virtual void Send(WebSocketOutMessage& message, Azure::Core::Context const& context) = 0;

      /**
       * @brief Set a callback to be called when a message is received from the server.
       *
       * @param handler A callback function which gets the incoming message from network.
       */
      virtual void OnMessage(std::function<void(WebSocketInMessage const&)> const& handler) = 0;

    protected:
      Azure::Core::Url m_url;
      WebSocketClientOptions m_clientOptions;
    };

  } // namespace _detail

  /**
   * @brief WebSocket client class provides network communication with a server, using websocket
   * protoccol.
   *
   */
  class WebSocketClient {
  private:
    std::unique_ptr<_detail::WebSocketClientImplementation> m_client;

  public:
    /**
     * @brief Construct a new WebSocket Client object.
     *
     * @param clientOptions Optional configuration used to create the websocket client.
     */
    explicit WebSocketClient(
        Azure::Core::Url url,
        WebSocketClientOptions clientOptions = WebSocketClientOptions{});

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
    void Send(WebSocketOutMessage& message, Azure::Core::Context const& context)
    {
      m_client->Send(message, context);
    }

    /**
     * @brief Set a callback to be called when a message is received from the server.
     *
     * @param handler A callback function which gets the incoming message from network.
     */
    void OnMessage(std::function<void(WebSocketInMessage const&)> const& handler)
    {
      m_client->OnMessage(handler);
    }
  };

}}} // namespace Azure::Core::WebSockets
