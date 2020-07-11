// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/policy.hpp>

#include <sstream>

using namespace Azure::Core::Http;

std::string const TelemetryPolicy::g_emptyApplicationId;

std::string TelemetryPolicy::BuildTelemetryId(
    std::string const& componentName,
    std::string const& componentVersion,
    std::string const& applicationId)
{
  // Spec: https://azure.github.io/azure-sdk/general_azurecore.html#telemetry-policy
  std::ostringstream telemetryId;

  if (!applicationId.empty())
  {
    telemetryId << applicationId.substr(0, 24) << " ";
  }

  telemetryId << "azsdk-cpp-" << componentName << "/" << componentVersion
#ifdef _az_SDK_PLATFORM_STRING
              << " (" _az_SDK_PLATFORM_STRING ")"
#endif
      ;

  return telemetryId.str();
}

std::unique_ptr<Response> TelemetryPolicy::Send(
    Context& ctx,
    Request& request,
    NextHttpPolicy nextHttpPolicy) const
{
  request.AddHeader("User-Agent", m_telemetryId);
  return nextHttpPolicy.Send(ctx, request);
}
