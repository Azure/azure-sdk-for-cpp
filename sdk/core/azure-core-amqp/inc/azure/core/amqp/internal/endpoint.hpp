// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#if ENABLE_UAMQP

#include "azure/core/amqp/models/amqp_value.hpp"
#include "common/async_operation_queue.hpp"
#include "connection_string_credential.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

extern "C"
{
  struct ENDPOINT_INSTANCE_TAG;
  struct LINK_ENDPOINT_INSTANCE_TAG;
}
namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class EndpointFactory;
  class LinkEndpointFactory;
}}}} // namespace Azure::Core::Amqp::_detail
namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  class ConnectionEvents;

  // An "Endpoint" is an intermediate type used to create sessions in an OnNewSession callback.
  class Endpoint final {
  public:
    ~Endpoint();
    Endpoint(Endpoint&& that) noexcept : m_endpoint{that.m_endpoint} { that.m_endpoint = nullptr; }

  private:
    ENDPOINT_INSTANCE_TAG* m_endpoint;
    Endpoint(ENDPOINT_INSTANCE_TAG* endpoint) : m_endpoint{endpoint} {};

    Endpoint(Endpoint const&) = delete;
    Endpoint& operator=(Endpoint const&) = delete;
    Endpoint& operator=(Endpoint&& other) noexcept;
    ENDPOINT_INSTANCE_TAG* Release()
    {
      ENDPOINT_INSTANCE_TAG* rv = m_endpoint;
      m_endpoint = nullptr;
      return rv;
    }
    friend class _detail::EndpointFactory;
    friend class _internal::ConnectionEvents;
  };

  // A "Link Endpoint" is an intermediate type used to create new Links in an OnLinkAttached
  // callback. Note that LinkEndpoints do not support copy semantics, and the only way to
  // retrieve the underlying LINK_ENDPOINT_INSTANCE_TAG is to call Release(). That is because
  // the primary use scenario for a LinkEndpoint is to call link_create_from_endpoint, and
  // link_create_from_endpoint takes ownership of the underlying LINK_ENDPOINT object.
  class LinkEndpoint final {
  public:
    LinkEndpoint(LinkEndpoint&& that) noexcept : m_endpoint{that.m_endpoint}
    {
      that.m_endpoint = nullptr;
    }
    ~LinkEndpoint() {};

    LINK_ENDPOINT_INSTANCE_TAG* Get() const { return m_endpoint; }
    std::uint32_t GetHandle() const;

  private:
    LINK_ENDPOINT_INSTANCE_TAG* m_endpoint;

    LinkEndpoint(LINK_ENDPOINT_INSTANCE_TAG* endpoint) : m_endpoint{endpoint} {};
    LINK_ENDPOINT_INSTANCE_TAG* Release()
    {
      LINK_ENDPOINT_INSTANCE_TAG* rv = m_endpoint;
      m_endpoint = nullptr;
      return rv;
    }

    /* NOTE: We do *NOT* own a LinkEndpoint object, it is completely controlled by uAMQP-c. As
     * such, we are not allowed to free it.*/
    LinkEndpoint(Endpoint const&) = delete;
    LinkEndpoint& operator=(LinkEndpoint const&) = delete;

    LinkEndpoint& operator=(Endpoint&& other);
    friend class _detail::LinkEndpointFactory;
  };
}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class EndpointFactory final {
  public:
    static _internal::Endpoint CreateEndpoint(ENDPOINT_INSTANCE_TAG* endpoint);
    static ENDPOINT_INSTANCE_TAG* Release(_internal::Endpoint& endpoint)
    {
      return endpoint.Release();
    }
  };

  class LinkEndpointFactory final {
  public:
    static _internal::LinkEndpoint CreateLinkEndpoint(LINK_ENDPOINT_INSTANCE_TAG* endpoint);
    static LINK_ENDPOINT_INSTANCE_TAG* Release(_internal::LinkEndpoint& linkEndpoint)
    {
      return linkEndpoint.Release();
    }
  };
}}}} // namespace Azure::Core::Amqp::_detail
#endif
