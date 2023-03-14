// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/credentials/credentials.hpp"

using Azure::Core::Credentials::TokenCredential;

std::string TokenCredential::GetCredentialName() const { return "Custom Credential"; }
