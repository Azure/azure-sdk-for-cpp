// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Helpers for identifying Event Hubs AMQP components in logs and tracing spans.
#pragma once

#include <azure/core/context.hpp>
#include <azure/core/diagnostics/logger.hpp>

#include <cstdint>
#include <exception>
#include <string>

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {

  struct AmqpDiagnosticsContext final
  {
    std::string ClientId;
    std::string PartitionId;
    std::string ComponentType;
    std::string ComponentName;
    std::uint64_t ComponentGeneration{};
  };

  // Includes the configured name when one exists and a UUID that remains stable for the lifetime
  // of the client.
  std::string CreateClientIdentifier(
      std::string const& clientType,
      std::string const& configuredName);

  // Returns the identifier shared by lifecycle logs and tracing spans.
  std::string GetAmqpComponentIdentifier(AmqpDiagnosticsContext const& diagnosticsContext);

  // Uses the AMQP error-condition namespace to identify the failed protocol layer. Conditions
  // without a connection or session scope are observed at the link operation.
  std::string GetAmqpFailureComponentType(std::string const& errorCondition);

  // Formats a lifecycle record. Kept separate from LogAmqpLifecycle so its stable fields can be
  // covered without replacing the process-wide logger listener in a unit test.
  std::string FormatAmqpLifecycleEvent(
      AmqpDiagnosticsContext const& diagnosticsContext,
      std::string const& eventName,
      std::string const& detail = {});

  void LogAmqpLifecycle(
      Azure::Core::Diagnostics::Logger::Level level,
      AmqpDiagnosticsContext const& diagnosticsContext,
      std::string const& eventName,
      std::string const& detail = {});

  // Keep recovery lifecycle reporting together so transport failures can be tested without a
  // service connection. The attach callback creates, opens, and installs the replacement receiver.
  template <typename CloseFunction, typename AttachFunction, typename LogFunction>
  std::uint64_t RebuildAmqpReceiver(
      AmqpDiagnosticsContext diagnosticsContext,
      std::uint64_t& nextReceiverGeneration,
      CloseFunction close,
      AttachFunction attach,
      LogFunction log)
  {
    // Close belongs to the installed receiver, even after a previous replacement failed.
    try
    {
      close();
    }
    catch (Azure::Core::OperationCancelledException const&)
    {
      throw;
    }
    catch (std::exception const& ex)
    {
      log(Azure::Core::Diagnostics::Logger::Level::Warning,
          diagnosticsContext,
          "close_failed",
          ex.what());
    }
    // Consume an ID for every replacement attempt, including unsuccessful attachments.
    diagnosticsContext.ComponentGeneration = ++nextReceiverGeneration;
    log(Azure::Core::Diagnostics::Logger::Level::Informational,
        diagnosticsContext,
        "reattaching",
        {});
    try
    {
      attach(diagnosticsContext);
      log(Azure::Core::Diagnostics::Logger::Level::Informational,
          diagnosticsContext,
          "reattached",
          {});
      return diagnosticsContext.ComponentGeneration;
    }
    catch (std::exception const& ex)
    {
      log(Azure::Core::Diagnostics::Logger::Level::Warning,
          diagnosticsContext,
          "reattach_failed",
          ex.what());
      throw;
    }
  }

  template <typename CloseFunction, typename LogFunction>
  void CloseAmqpComponent(
      AmqpDiagnosticsContext const& diagnosticsContext,
      bool discarded,
      std::string const& detail,
      CloseFunction close,
      LogFunction log)
  {
    log(discarded ? Azure::Core::Diagnostics::Logger::Level::Informational
                  : Azure::Core::Diagnostics::Logger::Level::Verbose,
        diagnosticsContext,
        discarded ? "discarded" : "closing",
        detail);
    try
    {
      close();
      if (!discarded)
      {
        log(Azure::Core::Diagnostics::Logger::Level::Verbose, diagnosticsContext, "closed", {});
      }
    }
    catch (std::exception const& ex)
    {
      log(Azure::Core::Diagnostics::Logger::Level::Warning,
          diagnosticsContext,
          "close_failed",
          ex.what());
    }
  }

}}}} // namespace Azure::Messaging::EventHubs::_detail
