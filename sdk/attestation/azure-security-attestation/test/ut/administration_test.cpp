// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/attestation/attestation_administration_client.hpp"
#include "azure/identity/client_secret_credential.hpp"
#include <azure/attestation/attestation_client_models.hpp>
#include <azure/core/test/test_base.hpp>
#include <cstdlib>
#include <gtest/gtest.h>
#include <string>
#include <tuple>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;

namespace Azure { namespace Security { namespace Attestation { namespace Test {

  class AdministrationTests
      : public Azure::Core::Test::TestBase,
        public testing::WithParamInterface<std::tuple<std::string, AttestationType>> {
  private:
  protected:
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;
    std::string m_endpoint;

    // Create
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);

      std::string const mode(std::get<0>(GetParam()));
      if (mode == "Shared")
      {
        std::string const shortLocation(GetEnv("LOCATION_SHORT_NAME"));
        m_endpoint = "https://shared" + shortLocation + "." + shortLocation + ".attest.azure.net";
      }
      else if (mode == "Aad")
      {
        m_endpoint = GetEnv("ATTESTATION_AAD_URL");
      }
      else if (mode == "Isolated")
      {
        m_endpoint = GetEnv("ATTESTATION_ISOLATED_URL");
      }
    }

    std::unique_ptr<AttestationAdministrationClient> CreateClient()
    {
      // `InitTestClient` takes care of setting up Record&Playback.
      Azure::Security::Attestation::AttestationAdministrationClientOptions options;
      if (m_testContext.IsPlaybackMode())
      {
        // Skip validating time stamps if using recordings.
        options.TokenValidationOptions.ValidateNotBeforeTime = false;
        options.TokenValidationOptions.ValidateExpirationTime = false;
      }
      char** env = environ;
      Azure::Core::Test::TestBase::TestLog("Dumping Environment variables");
      while (*env != nullptr)
      {
        std::stringstream ss;
        ss << "Env Variable " << *env << std::endl;
        Azure::Core::Test::TestBase::TestLog(ss.str());
        env += 1;
      }
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
          = std::make_shared<Azure::Identity::ClientSecretCredential>(
              GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

      return InitTestClient<
          Azure::Security::Attestation::AttestationAdministrationClient,
          Azure::Security::Attestation::AttestationAdministrationClientOptions>(
          m_endpoint, credential, options);
    }
  };

  TEST_P(AdministrationTests, GetPolicy)
  {
    auto adminClient(CreateClient());

    EXPECT_FALSE(adminClient->ClientVersion().empty());

    AttestationType attestationType(std::get<1>(GetParam()));
    {
      auto policy = adminClient->GetAttestationPolicy(attestationType);

      // The policy should have a value, and the token should have been issued by the service.
      if (policy.Value.Body.empty())
      {
        EXPECT_EQ(AttestationType::Tpm, attestationType);
      }
      else
      {
        EXPECT_EQ(0UL, policy.Value.Body.find("version"));
      }
      if (!m_testContext.IsPlaybackMode())
      {
        EXPECT_EQ(m_endpoint, policy.Value.Issuer.Value());
      }
    }

    {
      GetPolicyOptions gpOptions;
      EXPECT_FALSE(gpOptions.TokenValidationOptions);
    }
  }

  INSTANTIATE_TEST_SUITE_P(
      Attestation,
      AdministrationTests,
      testing::Combine(
          ::testing::Values("Shared", "Aad", "Isolated"),
          ::testing::Values(
              AttestationType::SgxEnclave,
              AttestationType::OpenEnclave,
              AttestationType::Tpm)),
      [](testing::TestParamInfo<AdministrationTests::ParamType> const& info) -> std::string {
        std::string instanceType = std::get<0>(info.param);
        return instanceType + "_" + std::get<1>(info.param).ToString();
      });

}}}} // namespace Azure::Security::Attestation::Test
