// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/dll_import_export.hpp"
#include "azure/core/amqp/models/amqp_header.hpp"
#include "azure/core/amqp/models/amqp_properties.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"

#include <azure/core/internal/extendable_enumeration.hpp>

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  class AmqpErrorCondition final
      : public Azure::Core::_internal::ExtendableEnumeration<AmqpErrorCondition> {
  public:
    AmqpErrorCondition() : ExtendableEnumeration(){};
    explicit AmqpErrorCondition(std::string const& value) : ExtendableEnumeration(value) {}

    /** @brief An AMQP Internal Error
     *
     * An internal error occurred. Operator intervention might be necessary to resume normal
     * operation.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition InternalError;

    /** @brief Entity Not Found
     * A peer attempted to work with a remote entity that does not
     * exist.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition NotFound;

    /** @brief Unauthorized Access.
     *
     * A peer attempted to work with a remote entity to which it has no access due to security
     * settings.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition UnauthorizedAccess;

    /** @brief Decoder Error
     * Data could not be decoded.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition DecodeError;

    /** @brief Resource allocation exceeded.
     *
     * A peer exceeded its resource allocation.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition ResourceLimitExceeded;

    /** @brief Not Allowed.
     *
     * The peer tried to use a frame in a manner that is inconsistent with the semantics defined
     * in the specification. For more information, see
     *
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition NotAllowed;

    /** @brief Invalid Field.
     *
     * An invalid field was passed in a frame body, and the operation could not proceed.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     *
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition InvalidField;

    /** @brief Not Implemented.
     * The peer tried to use functionality that is not implemented in its partner.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     *
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition NotImplemented;

    /** @brief Resource Locked.
     *
     * The client attempted to work with a server entity to which it has no access because
     * another client is working with it.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     *
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition ResourceLocked;

    /** @brief Precondition Failed.
     *
     * The client made a request that was not allowed because some precondition failed.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     *
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition PreconditionFailed;
    /** @brief Resource Deleted.
     *
     * A server entity the client is working with has been deleted.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     *
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition ResourceDeleted;

    /** @brief Illegal State.
     *
     * The peer sent a frame that is not permitted in the current state.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     *
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition IllegalState;

    /** @brief Frame Size Too Small.
     *
     * The peer cannot send a frame because the smallest encoding of the performative with the
     * currently valid values would be too large to fit within a frame of the agreed maximum
     * frame size. When transferring a message the message data can be sent in multiple transfer
     * frames thereby avoiding this error. Similarly when attaching a link with a large
     * unsettled map the endpoint MAY make use of the incomplete-unsettled flag to avoid the
     * need for overly large frames.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     *
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition FrameSizeTooSmall;

    /**
     * The link has been attached elsewhere, causing the existing attachment to be forcibly
     * closed.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition LinkStolen;

    /**
     * The peer sent a larger message than is supported on the link.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition LinkPayloadSizeExceeded;
    /**
     * An operator intervened to detach for some reason.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition LinkDetachForced;

    /**
     * An operator intervened to close the connection for some reason. The client could retry at
     * some later date.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition ConnectionForced;

    // These are errors that are specific to Azure services.
    /**
     * The server is busy.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition ServerBusyError;
    /**
     * One or more arguments supplied to the method are invalid.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition ArgumentError;
    /**
     * One or more arguments supplied to the method are invalid.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition ArgumentOutOfRangeError;
    /**
     * Request for a runtime operation on a disabled entity.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition EntityDisabledError;
    /**
     * Partition is not owned.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition PartitionNotOwnedError;
    /**
     * Lock token associated with the message or session has expired, or the lock token is not
     * found.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition StoreLockLostError;
    /**
     * The TokenProvider object could not acquire a token, the token is invalid, or the token
     * does not contain the claims required to perform the operation.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition PublisherRevokedError;
    /**
     * The server did not respond to the requested operation within the specified time. The
     * server may have completed the requested operation. This can happen due to network or
     * other infrastructure delays.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition TimeoutError;
    /**
     * Tracking Id for an exception.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition TrackingIdProperty;
    /**
     * IO exceptions that occur in proton-j library.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition ProtonIo;
    /**
     * A connection error occurred. A valid frame header cannot be formed from the incoming byte
     * stream.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition ConnectionFramingError;
    /**
     * The operation was cancelled.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition OperationCancelled;
    /**
     * Error condition when receiver attempts {@code complete}, {@code abandon}, {@code
     * renewLock}, {@code deadLetter}, or {@code defer} on a peek-locked message whose lock had
     * already expired.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition MessageLockLost;
    /**
     * Error condition when a session receiver performs an operation on a session after its lock
     * is expired. When a client accepts a session, the session is locked to the receiver for a
     * duration specified in the entity definition. When the accepted session remains idle for
     * the duration of lock, that is no operations performed on the session, the lock expires
     * and the session is made available to other clients.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition SessionLockLost;
    /**
     * Error condition when a client attempts to accept a session that is already locked by
     * another client.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition SessionCannotBeLocked;
    /**
     * Error condition when a receiver attempts to receive a message with sequence number and
     * the message with that sequence number is not available in the queue or subscription.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition MessageNotFound;
    /**
     * Error condition when a receiver attempts to receive from a session that does not exist.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition SessionNotFound;
    /**
     * Error condition when a subscription client tries to create a rule with the name of an
     * already existing rule.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition EntityAlreadyExists;

    /**
     * The container is no longer available on the current connection. The peer SHOULD attempt
     * reconnection to the container using the details provided in the info map.
     *
     * The address provided cannot be resolved to a terminus at the current container. The info
     * map MAY contain the following information to allow the client to locate the attach to the
     * terminus.
     *
     * hostname:
     * the hostname of the container. This is the value that SHOULD be supplied in the hostname
     * field of the open frame, and during the SASL and TLS negotiation (if used).
     *
     * network-host:
     * the DNS hostname or IP address of the machine hosting the container.
     *
     * port:
     * the port number on the machine hosting the container.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition ConnectionRedirect;

    /**
     * The address provided cannot be resolved to a terminus at the current container. The info
     * map MAY contain the following information to allow the client to locate the attach to the
     * terminus.
     *
     * hostname:
     * the hostname of the container hosting the terminus. This is the value that SHOULD be
     * supplied in the hostname field of the open frame, and during SASL and TLS negotiation (if
     * used).
     *
     * network-host:
     * the DNS hostname or IP address of the machine hosting the container.
     *
     * port:
     * the port number on the machine hosting the container.
     *
     * address:
     * the address of the terminus at the container.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition LinkRedirect;

    /**
     * The peer sent more message transfers than currently allowed on the link.
     */
    AZURE_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition TransferLimitExceeded;
  };

  struct AmqpError final
  {
    /** @brief Construct an AmqpError. */
    AmqpError() = default;

    /** @brief Destroy an AmqpError. */
    ~AmqpError() = default;

    /** @brief Copy Constructor */
    AmqpError(AmqpError const&) = default;

    /** @brief Assignment operator */
    AmqpError& operator=(AmqpError const&) = default;

    /** @brief Move Constructor */
    AmqpError(AmqpError&&) = default;

    /** @brief Move assignment operator */
    AmqpError& operator=(AmqpError&&) = default;

    /** @brief A symbolic value indicating the error condition.
     *
     * @remarks For more information, see [AMQP
     * Section 2.8.14](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-error).
     */
    AmqpErrorCondition Condition;

    /** @brief This text supplies any supplementary details not indicated by the condition
     * field. This text can be logged as an aid to resolving issues.
     *
     * @remarks For more information, see [AMQP
     * Section 2.8.14](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-error).
     */
    std::string Description;

    /** @brief A map containing information about the error condition.
     *
     * @remarks For more information, see [AMQP
     * Section 2.8.14](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-error).
     */
    AmqpMap Info;

    /** @brief Returns true if the AmqpError has a value.
     *
     * @return true if the AmqpError has a value.
     *
     */
    operator bool() const
    {
      return !(Condition.ToString().empty() && Description.empty() && Info.empty());
    }
  };
  std::ostream& operator<<(std::ostream&, AmqpError const&);

}}}}} // namespace Azure::Core::Amqp::Models::_internal
