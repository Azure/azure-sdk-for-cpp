// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/eventhubs_diagnostics.hpp"

#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/uuid.hpp>

#include <sstream>

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {

  namespace {
    std::string DisplayPartition(std::string const& partitionId)
    {
      return partitionId.empty() ? "<gateway>" : partitionId;
    }

    std::string DisplayComponentName(std::string const& componentName)
    {
      return componentName.empty() ? "<unnamed>" : componentName;
    }

    std::string EscapeLogValue(std::string const& value)
    {
      std::string escaped;
      escaped.reserve(value.size());
      for (auto const character : value)
      {
        switch (character)
        {
          case '\\':
            escaped += "\\\\";
            break;
          case '\'':
            escaped += "\\'";
            break;
          case '\n':
            escaped += "\\n";
            break;
          case '\r':
            escaped += "\\r";
            break;
          default:
            escaped += character;
        }
      }
      return escaped;
    }
  } // namespace

  std::string CreateClientIdentifier(
      std::string const& clientType,
      std::string const& configuredName)
  {
    return clientType + ":" + (configuredName.empty() ? std::string{} : configuredName + ":")
        + Azure::Core::Uuid::CreateUuid().ToString();
  }

  std::string GetAmqpComponentIdentifier(AmqpDiagnosticsContext const& diagnosticsContext)
  {
    std::ostringstream identifier;
    identifier << diagnosticsContext.ClientId << "/partition/"
               << DisplayPartition(diagnosticsContext.PartitionId) << "/generation/"
               << diagnosticsContext.ComponentGeneration << "/" << diagnosticsContext.ComponentType;
    return identifier.str();
  }

  std::string GetAmqpFailureComponentType(std::string const& errorCondition)
  {
    if (errorCondition.find("amqp:connection:") == 0)
    {
      return "connection";
    }
    if (errorCondition.find("amqp:session:") == 0)
    {
      return "session";
    }
    return "link";
  }

  std::string FormatAmqpLifecycleEvent(
      AmqpDiagnosticsContext const& diagnosticsContext,
      std::string const& eventName,
      std::string const& detail)
  {
    std::ostringstream message;
    message << "Event Hubs AMQP lifecycle: event='" << EscapeLogValue(eventName) << "' client.id='"
            << EscapeLogValue(diagnosticsContext.ClientId) << "' partition.id='"
            << EscapeLogValue(DisplayPartition(diagnosticsContext.PartitionId))
            << "' component.type='" << EscapeLogValue(diagnosticsContext.ComponentType)
            << "' component.name='"
            << EscapeLogValue(DisplayComponentName(diagnosticsContext.ComponentName))
            << "' component.id='" << EscapeLogValue(GetAmqpComponentIdentifier(diagnosticsContext))
            << "' component.generation=" << diagnosticsContext.ComponentGeneration;
    if (!detail.empty())
    {
      message << " detail='" << EscapeLogValue(detail) << "'";
    }
    return message.str();
  }

  void LogAmqpLifecycle(
      Azure::Core::Diagnostics::Logger::Level level,
      AmqpDiagnosticsContext const& diagnosticsContext,
      std::string const& eventName,
      std::string const& detail)
  {
    Azure::Core::Diagnostics::_internal::Log::Write(
        level, FormatAmqpLifecycleEvent(diagnosticsContext, eventName, detail));
  }

}}}} // namespace Azure::Messaging::EventHubs::_detail
