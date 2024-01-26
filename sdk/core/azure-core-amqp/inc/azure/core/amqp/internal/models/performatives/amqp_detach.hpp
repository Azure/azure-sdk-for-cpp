// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

//
// This file contains protocol level definitions for the AMQP protocol.

#pragma once

#include "azure/core/amqp/internal/models/amqp_error.hpp"

#include <cstdint>

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  namespace Performatives {

    /** Detach Performative (0x00000000:0x00000016)
     *
     * See
     * https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-detach
     * for more information.
     */
    class AmqpDetach {
    public:
      AmqpDetach() = default;

      uint32_t Handle{};
      bool Closed{};
      AmqpError Error;
    };
}}}}}} // namespace Azure::Core::Amqp::Models::_internal::Performatives