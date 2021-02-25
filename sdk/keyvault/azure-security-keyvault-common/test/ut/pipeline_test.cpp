// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include <azure/core/http/http.hpp>
#include <azure/core/http/policy.hpp>
#include <azure/core/internal/http/url.hpp>
#include <azure/keyvault/common/internal/keyvault_pipeline.hpp>

#include <memory>

using namespace Azure::Security::KeyVault::Common::Internal;

TEST(KeyVaultPipeline, initPipeline)
{
  std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>());
  Azure::Core::Internal::Http::Url url("urlTest");
  EXPECT_NO_THROW(KeyVaultPipeline p(url, "version", policies));
}
