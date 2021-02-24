// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/template/second/template_client.hpp"
#include "azure/template/second/version.hpp"

#include <string>

using namespace Azure::Template::Second;

std::string const TemplateClient::ClientVersion() { return Details::Version::VersionString(); }
