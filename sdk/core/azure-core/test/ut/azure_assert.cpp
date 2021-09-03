// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/azure_assert.hpp>

#include <gtest/gtest.h>

TEST(AzureAssert, AzureNoReturnPath) { ASSERT_DEATH(AzureNoReturnPath("test"), ""); }
