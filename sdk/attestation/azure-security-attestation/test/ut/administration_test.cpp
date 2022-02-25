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

  struct AttestationTestParam
  {
    std::string TypeNameString;
    AttestationType Type;
  };

  class AdministrationTests : public Azure::Core::Test::TestBase,
                              public testing::WithParamInterface<AttestationTestParam> {
  private:
  protected:
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;
    std::string m_endpoint;

    // Create
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);

      std::string const mode(GetParam().TypeNameString);
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

    AttestationType attestationType(GetParam().Type);
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

  // enum to control the output of the GetTestInput
  enum class TestCaseType
  {
    Get,
    SetNoSigned,
    SetSigned,
    Policy
  };

  std::vector<AttestationTestParam> GetTestInput(TestCaseType testCaseType)
  {
    std::vector<AttestationTestParam> returnCases;
    std::vector<std::string> typeNameList{"Isolated"}; // everyone support this
    if (testCaseType == TestCaseType::Get || testCaseType == TestCaseType::SetNoSigned)
    {
      typeNameList.emplace_back("AAD");
      if (testCaseType != TestCaseType::SetNoSigned)
      {
        typeNameList.emplace_back("Shared"); // only for GetPolicy
      }
    }
    for (auto const& value : typeNameList)
    {
      for (auto const& type :
           {AttestationType::SgxEnclave, AttestationType::OpenEnclave, AttestationType::Tpm})
      {
        returnCases.emplace_back(AttestationTestParam{value, type});
      }
    }
    return returnCases;
  }
  std::string GetTestName(testing::TestParamInfo<AdministrationTests::ParamType> const& info)
  {
    std::string instanceType = info.param.TypeNameString;
    return instanceType + "_" + info.param.Type.ToString();
  }

  INSTANTIATE_TEST_SUITE_P(
      AdministrationTestsGet,
      AdministrationTests,
      testing::ValuesIn(GetTestInput(TestCaseType::Get)),
      GetTestName);

  INSTANTIATE_TEST_SUITE_P(
      AdministrationTestsSetNoSigned,
      AdministrationTests,
      testing::ValuesIn(GetTestInput(TestCaseType::SetNoSigned)),
      GetTestName);

  INSTANTIATE_TEST_SUITE_P(
      AdministrationTestsSet,
      AdministrationTests,
      testing::ValuesIn(GetTestInput(TestCaseType::SetSigned)),
      GetTestName);

  INSTANTIATE_TEST_SUITE_P(
      AdministrationTestsPolicy,
      AdministrationTests,
      testing::ValuesIn(GetTestInput(TestCaseType::Policy)),
      GetTestName);

}}}} // namespace Azure::Security::Attestation::Test
