#pragma once

#include "transport.hpp"
#include <exception>

struct XIO_INSTANCE_TAG;

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Network {

  class AmqpHeaderTransport : public Transport {

  public:
    AmqpHeaderTransport(XIO_INSTANCE_TAG* parentTransport);
    ~AmqpHeaderTransport() = default;
  };

}}}}} // namespace Azure::Core::_internal::Amqp::Network
