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

    // Blackjack basic strategy vs dealer 10, 6+ decks, H17.
    if (key <= 0)
    {
      return 0;
    } // we were not dealt a hand
    else if (key > 21)
    {
      return -100;
    } // we busted
    else if (key == 21)
    {
      return 150;
    } // celebrate
    else if (key == 11)
    {
      return 20;
    } // double down
    else if (key < 11)
    {
      return 10;
    } // hit
    else if (key > 11 && key < 17)
    {
      return 1;
    } // hit, but be less happy about it
    else
    {
      return 0;
    } // >= 17 we always stay
  }
  catch (std::exception const& e)
  {
    tracingContext.Span.AddEvent(e);
    throw;
  }
}