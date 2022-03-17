// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "attestation_collateral.hpp"
#include "azure/attestation/attestation_administration_client.hpp"
#include "azure/attestation/attestation_client.hpp"
#include <azure/core/internal/json/json.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <gtest/gtest.h>
#include <tuple>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace Azure::Core;

namespace Azure { namespace Security { namespace Attestation { namespace Test {

  enum class InstanceType
  {
    Shared,
    AAD,
    Isolated
  };

  // cspell: words aikcert
  class TpmAttestationTests : public Azure::Core::Test::TestBase {
  private:
  protected:
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;

    // Create
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
    }

    std::string GetInstanceUri(InstanceType instanceType)
    {
      if (instanceType == InstanceType::Shared)
      {
        std::string shortLocation(GetEnv("LOCATION_SHORT_NAME"));
        return "https://shared" + shortLocation + "." + shortLocation + ".attest.azure.net";
      }
      else if (instanceType == InstanceType::AAD)
      {
        return GetEnv("ATTESTATION_AAD_URL");
      }
      else if (instanceType == InstanceType::Isolated)
      {
        return GetEnv("ATTESTATION_ISOLATED_URL");
      }
      throw std::runtime_error("Unkown instance type.");
    }

    std::unique_ptr<AttestationClient> CreateClient(InstanceType instanceType)
    {
      // `InitClientOptions` takes care of setting up Record&Playback.
      AttestationClientOptions options;
      if (m_testContext.IsPlaybackMode())
      {
        // Skip validating time stamps if using recordings.
        options.TokenValidationOptions.ValidateNotBeforeTime = false;
        options.TokenValidationOptions.ValidateExpirationTime = false;
      }
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
          = std::make_shared<Azure::Identity::ClientSecretCredential>(
              GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

      return InitTestClient<AttestationClient, AttestationClientOptions>(
          GetInstanceUri(instanceType), credential, options);
    }

    std::unique_ptr<AttestationAdministrationClient> CreateAdminClient(InstanceType instanceType)
    {
      // `InitTestClient` takes care of setting up Record&Playback.
      Azure::Security::Attestation::AttestationAdministrationClientOptions options;
      if (m_testContext.IsPlaybackMode())
      {
        // Skip validating time stamps if using recordings.
        options.TokenValidationOptions.ValidateNotBeforeTime = false;
        options.TokenValidationOptions.ValidateExpirationTime = false;
      }
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
          = std::make_shared<Azure::Identity::ClientSecretCredential>(
              GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

      return InitTestClient<
          Azure::Security::Attestation::AttestationAdministrationClient,
          Azure::Security::Attestation::AttestationAdministrationClientOptions>(
          GetInstanceUri(instanceType), credential, options);
    }
  };

  TEST_F(TpmAttestationTests, AttestTpm)
  {
    // TPM attestation requires a policy document be set. For simplicity, we only run the
    // test against an AAD attestation service instance.
    {
      auto adminClient(CreateAdminClient(InstanceType::AAD));
      // Set a minimal policy, which will make the TPM attestation code happy.
      adminClient->SetAttestationPolicy(
          AttestationType::Tpm, "version=1.0; authorizationrules{=> permit();}; issuancerules{};");
    }

    auto client(CreateClient(InstanceType::AAD));

    auto response = client->AttestTpm(R"({"payload": { "type": "aikcert" } })");

    Azure::Core::Json::_internal::json parsedResponse(
        Azure::Core::Json::_internal::json::parse(response.Value));
    EXPECT_TRUE(parsedResponse.contains("payload"));
    EXPECT_TRUE(parsedResponse["payload"].is_object());
    EXPECT_TRUE(parsedResponse["payload"].contains("challenge"));
    EXPECT_TRUE(parsedResponse["payload"].contains("service_context"));

    {
      auto adminClient(CreateAdminClient(InstanceType::AAD));
      adminClient->RetrieveResponseValidationCollateral();
      // Clean up after setting the policy.
      adminClient->ResetAttestationPolicy(AttestationType::Tpm);
    }
  }

}}}} // namespace Azure::Security::Attestation::Test
