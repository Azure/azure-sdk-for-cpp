// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/template/template_client.hpp"

#include "private/package_version.hpp"

#include <string>

using namespace Azure::Template;
using namespace Azure::Template::_detail;

std::string TemplateClient::ClientVersion() const { return PackageVersion::ToString(); }

TemplateClient::TemplateClient(TemplateClientOptions options)
{
}

int TemplateClient::GetValue(int key) const
{
  if (key < 0)
  {
    return 0;
  }

  return key + 1;
}
