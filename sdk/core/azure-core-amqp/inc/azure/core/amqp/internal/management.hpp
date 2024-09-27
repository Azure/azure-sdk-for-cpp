// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/models/amqp_error.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "session.hpp"

#include <azure/core/context.hpp>

#include <string>
#include <tuple>
#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class ManagementClientImpl;
  class ManagementClientFactory;
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  enum class ManagementOperationStatus
  {
    Invalid,
    Ok,
    Error,
    FailedBadStatus,
    InstanceClosed,
    Cancelled,
  };

  enum class ManagementOpenStatus
  {
    Invalid,
    Ok,
    Error,
    Cancelled,
  };

  struct ManagementClientOptions final
  {
    /**
     * @brief Expected status code key name.
     *
     * Specifies the key name which will contain the result of the management operation.
     *
     * @remarks AMQP management plane operations return the status code and description in the
     * ApplicationProperties field of the AmqpMessage. By default, the error code property is in
     * a key named "statusCode", this allows a client of the Management object to override the
     * default value with one specific to the management API being called.
     *
     * For more information, see [Service Bus Request
     * Response](https://learn.microsoft.com/azure/service-bus-messaging/service-bus-amqp-request-response#response).
     */
    std::string ExpectedStatusCodeKeyName = "statusCode";

    /**
     * @brief Expected status description key name.
     *
     * Specifies the key name which will contain the description of the result of the management
     * operation.
     *
     * @remarks AMQP management plane operations return the status code and description in the
     * ApplicationProperties field of the AmqpMessage. By default, the error description
     * property is in a key named "statusDescription", this allows a client of the Management
     * object to override the default value with one specific to the management API being
     * called.
     *
     * For more information, see [Service Bus Request
     * Response](https://learn.microsoft.com/azure/service-bus-messaging/service-bus-amqp-request-response#response).
     */
    std::string ExpectedStatusDescriptionKeyName = "statusDescription";

    /** @brief The name of the management node.
     *
     * By default, the name of the management node is "$management", but under certain
     * circumstances, management operations can be performed on a different node (for instance,
     * $cbs for claims based authentication)
     */
    std::string ManagementNodeName = "$management";

    /**
     * @brief Enable trace logging for the management operations.
     */
    bool EnableTrace{false};
  };

#if ENABLE_RUST_AMQP
  /**
   * @brief Callback event handler for management events such as error.
   */
  class ManagementClientEvents {
  protected:
    ~ManagementClientEvents() {}

  public:
    /** @brief Called when an error occurs.
     *
     * @param error - the error which occurred.
     *
     */
    virtual void OnError(Models::_internal::AmqpError const& error) = 0;
  };
#endif

  /**
   * @brief Result of a management operation.
   */
  struct ManagementOperationResult final
  {
    /**
     * @brief The status of the operation.
     */
    ManagementOperationStatus Status = ManagementOperationStatus::Invalid;

    /**
     * @brief The response message from the operation, if Status is ManagementOperationStatus::Ok.
     */
    std::shared_ptr<Models::AmqpMessage> Message;

    /**
     * @brief The error code associated with the message, if Status is
     * ManagementOperationStatus::Error.
     */
    Models::_internal::AmqpError Error;

    /**
     * @brief The HTTP status code of the operation, if Status is ManagementOperationStatus::Error.
     */
    uint32_t StatusCode = 0;
  };

  /** @brief AMQP Management APIs.
   *
   * The AMQP management plane is a set of APIs that allow for management operations to be
   * performed on an AMQP connection. See [AMQP Management
   * Version 1.0](https://www.oasis-open.org/committees/download.php/52425/amqp-man-v1%200-wd08.pdf)
   * for more information.
   *
   */
  class ManagementClient final {
  public:
    /**
     * @brief Represents a client used to manage AMQP entities.
     */
    ManagementClient() = default;

    /**
     * @brief Destructor for the ManagementClient class.
     */
    ~ManagementClient() noexcept = default;

    /**
     * @brief Copy constructor.
     */
    ManagementClient(ManagementClient const&) = default;

    /**
     * @brief Assignment operator.
     */
    ManagementClient& operator=(ManagementClient const&) = default;

    /**
     * @brief Move constructor.
     */
    ManagementClient(ManagementClient&&) = default;

    /**
     * @brief Move assignment operator.
     */
    ManagementClient& operator=(ManagementClient&&) = default;

    /**
     * @brief Open the management instance.
     *
     * @returns The result of the open operation.
     */
    _azure_NODISCARD ManagementOpenStatus Open(Context const& context = {});

    /**
     * @brief Close the management instance.
     *
     * @param context Context for the operation.
     */
    void Close(Context const& context = {});

    /**
     * @brief Execute a management operation.
     *
     * @param operationToPerform - the operation to perform (case sensitive).
     * @param typeOfOperation - the type of operation (case sensitive).
     * @param locales - the locales to use - A list of locales that the sending peer permits for
     * incoming informational text in response messages. This value MUST be of the form presented
     * in the "Language-Tag" rule of [RFC2616], section 3.10.
     * @param messageToSend - the message to send.
     * @param context - the context for the operation.
     *
     * @returns a ManagementOperationResult which includes the high level result of the operation,
     * the HTTP response status code, the status description, and the response message.
     *
     * @remark The messageToSend is intentionally passed by value because the ExecuteOperation needs
     * to modify the message to add the required properties for the management operation.
     */
    _azure_NODISCARD ManagementOperationResult ExecuteOperation(
        std::string const& operationToPerform,
        std::string const& typeOfOperation,
        std::string const& locales,
        Models::AmqpMessage messageToSend,
        Context const& context = {});

  private:
    friend class Azure::Core::Amqp::_detail::ManagementClientFactory;
    ManagementClient(std::shared_ptr<_detail::ManagementClientImpl> impl) : m_impl{impl} {}
    std::shared_ptr<_detail::ManagementClientImpl> m_impl;
  };

}}}} // namespace Azure::Core::Amqp::_internal
