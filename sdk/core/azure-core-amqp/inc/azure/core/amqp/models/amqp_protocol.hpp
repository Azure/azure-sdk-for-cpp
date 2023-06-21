// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

//
// This file contains protocol level definitions for the AMQP protocol.

#pragma once

#include "azure/core/amqp/doxygen_pragma.hpp"

#include <azure/core/internal/unique_handle.hpp>

#include <cstdint>

struct AMQPVALUE_DECODER_HANDLE_DATA_TAG;

BEGIN_UNKNOWN_DOCUMENTATION_DIAGNOSTIC_IGNORE
/// @cond INTERNAL
template <> struct Azure::Core::_internal::UniqueHandleHelper<AMQPVALUE_DECODER_HANDLE_DATA_TAG>
{
  static void FreeAmqpDecoder(AMQPVALUE_DECODER_HANDLE_DATA_TAG* obj);

  using type = Azure::Core::_internal::
      BasicUniqueHandle<AMQPVALUE_DECODER_HANDLE_DATA_TAG, FreeAmqpDecoder>;
};
/// @endcond
END_UNKNOWN_DOCUMENTATION_DIAGNOSTIC_IGNORE

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  using UniqueAmqpDecoderHandle
      = ::Azure::Core::_internal::UniqueHandleHelper<AMQPVALUE_DECODER_HANDLE_DATA_TAG>::type;

  /** @brief AMQP Descriptor values. Note that the AMQP descriptor is technically a tuple of
   * domain+id, the domain for internal-to-amqp is defined to be 0x00000000.
   *
   * See [AMQP Descriptor
   * values](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#section-descriptor-values)
   * for more information.
   */
  enum class AmqpDescriptors : std::int64_t
  {
    // AMQP Performative descriptors.
    Error = 0x1d,
    Open = 0x10,
    Begin = 0x11,
    Attach = 0x12,
    Flow = 0x13,
    Transfer = 0x14,
    Disposition = 0x15,
    Detach = 0x16,
    End = 0x17,
    Close = 0x18,

    // Message Dispositions.
    Received = 0x23,
    Accepted = 0x24,
    Rejected = 0x25,
    Released = 0x26,
    Modified = 0x27,

    // Terminus related descriptors.
    Source = 0x28,
    Target = 0x29,

    // AMQP Sasl descriptors.
    SaslMechanism = 0x40,
    SaslInit = 0x41,
    SaslChallenge = 0x42,
    SaslResponse = 0x43,
    SaslOutcome = 0x44,

    // Message related descriptors.
    Header = 0x70,
    DeliveryAnnotations = 0x71,
    MessageAnnotations = 0x72,
    Properties = 0x73,
    ApplicationProperties = 0x74,
    DataBinary = 0x75,
    DataAmqpSequence = 0x76,
    DataAmqpValue = 0x77,
    Footer = 0x78,
  };

}}}} // namespace Azure::Core::Amqp::_detail
