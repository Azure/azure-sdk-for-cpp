// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/tracing/tracing.hpp"
#include "azure/core/context.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/tracing/service_tracing.hpp"
#include "azure/core/platform.hpp"
#include <cctype>
#include <sstream>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <windows.h>

#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.

namespace Azure { namespace Core { namespace Tracing { namespace _internal {

  /**
   * @brief HkeyHolder ensures native handle resource is released.
   *
   */
  class HkeyHolder final {
  private:
    HKEY m_value = nullptr;

  public:
    explicit HkeyHolder() noexcept : m_value(nullptr) {}

    ~HkeyHolder() noexcept
    {
      if (m_value != nullptr)
      {
        ::RegCloseKey(m_value);
      }
    }

    void operator=(HKEY p) noexcept
    {
      if (p != nullptr)
      {
        m_value = p;
      }
    }

    operator HKEY() noexcept { return m_value; }

    operator HKEY*() noexcept { return &m_value; }

    HKEY* operator&() noexcept { return &m_value; }
  };

}}}} // namespace Azure::Core::Tracing::_internal

#endif

#elif defined(AZ_PLATFORM_POSIX)
#include <sys/utsname.h>
#endif

namespace {
std::string GetOSVersion()
{
  std::ostringstream osVersionInfo;

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.
  {
    Azure::Core::Tracing::_internal::HkeyHolder regKey;
    if (RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
            0,
            KEY_READ,
            &regKey)
        == ERROR_SUCCESS)
    {
      auto first = true;
      static constexpr char const* regValues[]{
          "ProductName", "CurrentVersion", "CurrentBuildNumber", "BuildLabEx"};
      for (auto regValue : regValues)
      {
        char valueBuf[200] = {};
        DWORD valueBufSize = sizeof(valueBuf);

        if (RegQueryValueExA(regKey, regValue, NULL, NULL, (LPBYTE)valueBuf, &valueBufSize)
            == ERROR_SUCCESS)
        {
          if (valueBufSize > 0)
          {
            osVersionInfo << (first ? "" : " ")
                          << std::string(valueBuf, valueBuf + (valueBufSize - 1));
            first = false;
          }
        }
      }
    }
  }
#else
  {
    osVersionInfo << "UWP";
  }
#endif
#elif defined(AZ_PLATFORM_POSIX)
  {
    utsname sysInfo{};
    if (uname(&sysInfo) == 0)
    {
      osVersionInfo << sysInfo.sysname << " " << sysInfo.release << " " << sysInfo.machine << " "
                    << sysInfo.version;
    }
  }
#endif

  return osVersionInfo.str();
}

std::string TrimString(std::string s)
{
  auto const isSpace = [](int c) { return !std::isspace(c); };

  s.erase(s.begin(), std::find_if(s.begin(), s.end(), isSpace));
  s.erase(std::find_if(s.rbegin(), s.rend(), isSpace).base(), s.end());

  return s;
}
} // namespace
namespace Azure { namespace Core { namespace Tracing { namespace _internal {

  const SpanKind SpanKind::Internal("Internal");
  const SpanKind SpanKind::Client("Client");
  const SpanKind SpanKind::Consumer("Consumer");
  const SpanKind SpanKind::Producer("Producer");
  const SpanKind SpanKind::Server("Server");

  const SpanStatus SpanStatus::Unset("Unset");
  const SpanStatus SpanStatus::Ok("Ok");
  const SpanStatus SpanStatus::Error("Error");

  const TracingAttributes TracingAttributes::AzNamespace("az.namespace");
  const TracingAttributes TracingAttributes::ServiceRequestId("serviceRequestId");
  const TracingAttributes TracingAttributes::HttpUserAgent("http.user_agent");
  const TracingAttributes TracingAttributes::HttpMethod("http.method");
  const TracingAttributes TracingAttributes::HttpUrl("http.url");
  const TracingAttributes TracingAttributes::RequestId("requestId");
  const TracingAttributes TracingAttributes::HttpStatusCode("http.status_code");

  std::string TracingContextFactory::BuildUserAgent(
      std::string const& componentName,
      std::string const& componentVersion,
      std::string const& applicationId)
  {
    // Spec: https://azure.github.io/azure-sdk/general_azurecore.html#telemetry-policy
    std::ostringstream telemetryId;

    if (!applicationId.empty())
    {
      telemetryId << TrimString(applicationId).substr(0, 24) << " ";
    }

    static std::string const osVer = GetOSVersion();
    telemetryId << "azsdk-cpp-" << componentName << "/" << componentVersion << " (" << osVer << ")";

    return telemetryId.str();
  }

  using Azure::Core::Context;

  TracingContextFactory::TracingContext TracingContextFactory::CreateTracingContext(
      std::string const& methodName,
      Azure::Core::Context const& context) const
  {
    Azure::Core::Context contextToUse = context;
    CreateSpanOptions createOptions;

    createOptions.Kind = SpanKind::Internal;
    if (HasTracer())
    {
      createOptions.Attributes = m_serviceTracer->CreateAttributeSet();
    }
    return CreateTracingContext(methodName, createOptions, context);
  }

  TracingContextFactory::TracingContext TracingContextFactory::CreateTracingContext(
      std::string const& methodName,
      Azure::Core::Tracing::_internal::CreateSpanOptions& createOptions,
      Azure::Core::Context const& context) const
  {
    Azure::Core::Context contextToUse = context;

    // Ensure that the factory is available in the context chain.
    // Note that we do this even if we don't have distributed tracing enabled, that's because
    // the tracing context factory is also responsible for creating the User-Agent HTTP header, so
    // it needs to be available for all requests.
    TracingContextFactory const* tracingFactoryFromContext;
    if (!context.TryGetValue(TracingFactoryContextKey, tracingFactoryFromContext))
    {
      contextToUse = context.WithValue(TracingFactoryContextKey, this);
    }

    if (HasTracer())
    {
      std::shared_ptr<Span> traceContext;
      // Find a span in the context hierarchy.
      if (contextToUse.TryGetValue(ContextSpanKey, traceContext))
      {
        createOptions.ParentSpan = traceContext;
      }
      else
      {
        // Please note: Not specifically needed, but make sure that this is a root level
        // span if there is no parent span in the context
        createOptions.ParentSpan = nullptr;
      }

      if (!createOptions.Attributes)
      {
        createOptions.Attributes = m_serviceTracer->CreateAttributeSet();
      }
      createOptions.Attributes->AddAttribute(
          TracingAttributes::AzNamespace.ToString(), m_serviceName);

      std::shared_ptr<Span> newSpan(m_serviceTracer->CreateSpan(methodName, createOptions));
      Azure::Core::Context newContext = contextToUse.WithValue(ContextSpanKey, newSpan);
      ServiceSpan newServiceSpan(newSpan);
      return TracingContext{std::move(newContext), std::move(newServiceSpan)};
    }
    else
    {
      return TracingContext{contextToUse, ServiceSpan{}};
    }
  }

  std::unique_ptr<TracingContextFactory> TracingContextFactory::CreateFromContext(
      Azure::Core::Context const& context)
  {
    TracingContextFactory const* factory;
    if (context.TryGetValue(TracingFactoryContextKey, factory))
    {
      return std::make_unique<TracingContextFactory>(*factory);
    }
    else
    {
      return nullptr;
    }
  }

  std::unique_ptr<Azure::Core::Tracing::_internal::AttributeSet>
  TracingContextFactory::CreateAttributeSet() const
  {
    if (m_serviceTracer)
    {
      return m_serviceTracer->CreateAttributeSet();
    }
    return nullptr;
  }

  Azure::Core::Context::Key TracingContextFactory::ContextSpanKey;
  Azure::Core::Context::Key TracingContextFactory::TracingFactoryContextKey;
}}}} // namespace Azure::Core::Tracing::_internal
