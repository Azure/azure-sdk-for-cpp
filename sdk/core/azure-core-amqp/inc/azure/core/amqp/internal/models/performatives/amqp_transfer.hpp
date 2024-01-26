// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/dll_import_export.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "azure/core/nullable.hpp"

#include <azure/core/internal/extendable_enumeration.hpp>

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  namespace Performatives {

    struct AmqpTransfer final
    {
      /** @brief Construct an AmqpError. */
      AmqpTransfer() = default;

      /** @brief Destroy an AmqpError. */
      ~AmqpTransfer() = default;

      /** @brief Copy Constructor */
      AmqpTransfer(AmqpTransfer const&) = default;

      /** @brief Assignment operator */
      AmqpTransfer& operator=(AmqpTransfer const&) = default;

      /** @brief Move Constructor */
      AmqpTransfer(AmqpTransfer&&) = default;

      /** @brief Move assignment operator */
      AmqpTransfer& operator=(AmqpTransfer&&) = default;

      /** @brief The link on which the message is transferred.
       *
       * @remarks For more information, see [AMQP
       * Section 2.7.5](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-transfer).
       */
      std::uint32_t HandleValue{};

      /** @brief The Delivery ID for the message.
       *
       * @remarks For more information, see [AMQP
       * Section 2.7.5](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-transfer).
       */
      Azure::Nullable<std::uint32_t> DeliveryId;

      /** @brief The delivery tag. Uniquely identifies a delivery attempt for a given message on
       * this link.
       *
       * @remarks For more information, see [AMQP
       * Section 2.7.5](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-transfer).
       */
      AmqpBinaryData DeliveryTag{};

      /** @brief The message format code.
       *
       * @remarks For more information, see [AMQP
       * Section 2.7.5](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-transfer).
       */
      std::uint32_t MessageFormat{};

      /** @brief The settled state on the message.
       *
       * @remarks For more information, see [AMQP
       * Section 2.7.5](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-transfer).
       */
      bool Settled{};
      /** @brief Indicates that the message has more content.
       *
       * @remarks For more information, see [AMQP
       * Section 2.7.5](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-transfer).
       */
      bool More{};

      /** @brief Indicates the settle mode for the message.
       *
       * @remarks For more information, see [AMQP
       * Section 2.7.5](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-transfer).
       */
      std::uint32_t ReceiverSettleMode{};

      /** @brief The state of the delivery at the sender.
       *
       * @remarks For more information, see [AMQP
       * Section 2.7.5](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-transfer).
       */
      AmqpValue State;

      /** @brief Indicates a resumed delivery
       *
       * @remarks For more information, see [AMQP
       * Section 2.7.5](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-transfer).
       *
       */
      bool Resume{};

      /** @brief Indicates that the messgae is aborted.
       *
       * @remarks For more information, see [AMQP
       * Section 2.7.5](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-transfer).
       *
       */
      bool Aborted{};

      /** @brief Batchable hint..
       *
       * @remarks For more information, see [AMQP
       * Section 2.7.5](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-transfer).
       *
       */

      bool Batchable{};
    };
    std::ostream& operator<<(std::ostream&, AmqpTransfer const&);

}}}}}} // namespace Azure::Core::Amqp::Models::_internal::Performatives
