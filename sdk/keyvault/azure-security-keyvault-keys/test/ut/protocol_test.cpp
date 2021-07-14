// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/client_options.hpp>

#include "./../../src/private/keyvault_protocol.hpp"

#include <memory>

using namespace Azure::Security::KeyVault::_detail;

TEST(KeyVaultProtocolClient, initPipeline)
{
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
  policies.emplace_back(
      std::make_unique<Azure::Core::Http::Policies::_internal::TransportPolicy>());
  Azure::Core::Url url("urlTest");
  Azure::Core::_internal::ClientOptions options;
  Azure::Core::Http::_internal::HttpPipeline pipeline(
      options, "service-name", "service-version", std::move(policies), {});
  EXPECT_NO_THROW(KeyVaultProtocolClient p(url, "version", std::move(pipeline)));
}
