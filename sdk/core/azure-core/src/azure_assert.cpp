// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/azure_assert.hpp"

void AzureAssertNoReturn(bool exp, std::string msg) { AZURE_ASSERT_MSG(exp, msg); }
