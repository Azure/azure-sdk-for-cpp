// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/template/template_client.hpp"

#include "private/package_version.hpp"

#include <string>

using namespace Azure::Template;
using namespace Azure::Template::_detail;

std::string TemplateClient::ClientVersion() const { return PackageVersion::ToString(); }

int TemplateClient::GetValue(int key) const { return key < 0 ? 0 : key; }
