// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "common/async_operation_queue.hpp"
#include "connection_string_credential.hpp"
#include "models/amqp_value.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

extern "C"
{
  struct ENDPOINT_INSTANCE_TAG;
  struct LINK_ENDPOINT_INSTANCE_TAG;
}

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  // An "Endpoint" is an intermediate type used to create sessions in an OnNewSession callback.
  struct Endpoint
  {
    ENDPOINT_INSTANCE_TAG* m_endpoint;
    Endpoint(ENDPOINT_INSTANCE_TAG* endpoint) : m_endpoint{endpoint} {};
    ~Endpoint();
    Endpoint(Endpoint const&) = delete;
    Endpoint& operator=(Endpoint const&) = delete;

    Endpoint(Endpoint&& other) noexcept : m_endpoint{other.m_endpoint}
    {
      other.m_endpoint = nullptr;
    }
    Endpoint& operator=(Endpoint&& other);
    ENDPOINT_INSTANCE_TAG* Release()
    {
      ENDPOINT_INSTANCE_TAG* rv = m_endpoint;
      m_endpoint = nullptr;
      return rv;
    }
  };

  // A "Link Endpoint" is an intermediate type used to create new Links in an OnLinkAttached
  // callback. Note that LinkEndpoints do not support copy semantics, and the only way to
  // retrieve the underlying LINK_ENDPOINT_INSTANCE_TAG is to call Release(). That is because
  // the primary use scenario for a LinkEndpoint is to call link_create_from_endpoint, and
  // link_create_from_endpoint takes ownership of the underlying LINK_ENDPOINT object.
  struct LinkEndpoint
  {
    LINK_ENDPOINT_INSTANCE_TAG* m_endpoint;
    LinkEndpoint(LINK_ENDPOINT_INSTANCE_TAG* endpoint) : m_endpoint{endpoint} {};
    /* NOTE: We do *NOT* own a LinkEndpoint object, it is completely controlled by uAMQP-c. As
     * such, we are not allowed to free it.*/
    ~LinkEndpoint(){};
    LinkEndpoint(Endpoint const&) = delete;
    LinkEndpoint& operator=(LinkEndpoint const&) = delete;

    LinkEndpoint(LinkEndpoint&& other) noexcept : m_endpoint{other.m_endpoint}
    {
      other.m_endpoint = nullptr;
    }
    LinkEndpoint& operator=(Endpoint&& other);
    LINK_ENDPOINT_INSTANCE_TAG* Release()
    {
      LINK_ENDPOINT_INSTANCE_TAG* rv = m_endpoint;
      m_endpoint = nullptr;
      return rv;
    }
  };
}}}} // namespace Azure::Core::Amqp::_internal
