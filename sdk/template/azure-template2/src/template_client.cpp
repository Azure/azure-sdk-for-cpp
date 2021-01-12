// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/template2/template_client.hpp"
#include "azure/template2/version.hpp"

#include <string>

using namespace Azure::Template;

std::string const Template2Client::ClientVersion() { return Details::Version::VersionString(); }
