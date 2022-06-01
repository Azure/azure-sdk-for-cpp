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
  auto contextAndSpan = m_tracingFactory.CreateSpan(
      "GetValue", Azure::Core::Tracing::_internal::SpanKind::Internal, context);

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
    contextAndSpan.second.AddEvent(e);
    contextAndSpan.second.SetStatus(Azure::Core::Tracing::_internal::SpanStatus::Error);
  }
}
