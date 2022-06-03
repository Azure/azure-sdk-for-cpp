// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/template/template_client.hpp"

#include "private/package_version.hpp"

#include <string>

using namespace Azure::Template;
using namespace Azure::Template::_detail;

TemplateClient::TemplateClient(TemplateClientOptions const& options)
    : m_tracingFactory(options, "Template", PackageVersion::ToString())
{
}

int TemplateClient::GetValue(int key, Azure::Core::Context const& context) const
{
  auto tracingContext = m_tracingFactory.CreateTracingContext("GetValue", context);

  try
  {

    if (key < 0)
    {
      return 0;
    }

    return key + 1;
  }
  catch (std::exception const& e)
  {
    tracingContext.Span.AddEvent(e);
    tracingContext.Span.SetStatus(Azure::Core::Tracing::_internal::SpanStatus::Error);
  }
}
