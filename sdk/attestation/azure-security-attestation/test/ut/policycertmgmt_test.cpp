// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "../src/private/crypto/inc/crypto.hpp"
#include "attestation_collateral.hpp"
#include "azure/attestation/attestation_administration_client.hpp"
#include "azure/identity/client_secret_credential.hpp"
#include <azure/attestation/attestation_client_models.hpp>
#include <azure/core/internal/cryptography/sha_hash.hpp>
#include <azure/core/test/test_base.hpp>
#include <cstdlib>
#include <gtest/gtest.h>
#include <string>
#include <tuple>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace Azure::Security::Attestation::_detail;

namespace Azure { namespace Security { namespace Attestation { namespace Test {

  enum class ServiceInstanceType
  {
    Shared,
    AAD,
    Isolated
  };

  class CertificateTests : public Azure::Core::Test::TestBase,
                           public testing::WithParamInterface<ServiceInstanceType> {
  private:
  protected:
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;
    std::string m_endpoint;

    // Create
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
      ServiceInstanceType type = GetParam();
      if (type == ServiceInstanceType::Shared)
      {
        std::string const shortLocation(GetEnv("LOCATION_SHORT_NAME"));
        m_endpoint = "https://shared" + shortLocation + "." + shortLocation + ".attest.azure.net";
      }
      else if (type == ServiceInstanceType::AAD)
      {
        m_endpoint = GetEnv("ATTESTATION_AAD_URL");
      }
      else if (type == ServiceInstanceType::Isolated)
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

  public:
  }; // namespace Test

  // Get Policy management certificates for each instance type.
  // The GetPolicyManagementCertificates API can be run against all instance types, but it only
  // returns values on isolated instances (an isolated instance is defined to be an attestaiton
  // service instance with policy management certificates).
  TEST_P(CertificateTests, GetPolicyManagementCertificates)
  {
    auto adminClient(CreateClient());

    {
      auto certificatesResult = adminClient->GetPolicyManagementCertificates();

      // Do we expect to get any certificates in the response? AAD and Shared instances will never
      // have any certificates.
      bool expectedCertificates = false;
      if (GetParam() == ServiceInstanceType::Isolated)
      {
        expectedCertificates = true;
      }

      if (expectedCertificates)
      {
        ASSERT_NE(0, certificatesResult.Value.Body.Certificates.size());
      }
      else
      {
        ASSERT_EQ(0, certificatesResult.Value.Body.Certificates.size());
      }

      // In playback mode, the endpoint is a mocked value so the Issuer in the result will not
      // match.
      if (!m_testContext.IsPlaybackMode())
      {
        EXPECT_EQ(m_endpoint, *certificatesResult.Value.Issuer);

        if (expectedCertificates)
        {
          // Scan through the list of policy management certificates - the provisioned certificate
          // MUST be one of the returned certificates.
          //
          // In playback mode, the ISOLATED_SIGNING_CERTIFICATE environment variable is
          // mocked, so it cannot be parsed.

          bool foundIsolatedCertificate = false;
          auto isolatedCertificateBase64(GetEnv("ISOLATED_SIGNING_CERTIFICATE"));
          auto isolatedCertificate(Cryptography::ImportX509Certificate(
                  Cryptography::PemFromBase64(isolatedCertificateBase64, "CERTIFICATE")));
          for (const auto& signer : certificatesResult.Value.Body.Certificates)
          {
            auto signerCertificate
                = Cryptography::ImportX509Certificate(((*signer.CertificateChain)[0]));
            if (signerCertificate->GetThumbprint() == isolatedCertificate->GetThumbprint())
            {
              foundIsolatedCertificate = true;
            }
          }
          EXPECT_TRUE(foundIsolatedCertificate);
        }
      }
    }
  }

  INSTANTIATE_TEST_SUITE_P(
      PolicyCertificates,
      CertificateTests,
      testing::ValuesIn(
          {ServiceInstanceType::AAD, ServiceInstanceType::Shared, ServiceInstanceType::Isolated}),
      [](testing::TestParamInfo<CertificateTests::ParamType> const& testInfo) {
        switch (testInfo.param)
        {
          case ServiceInstanceType::AAD:
            return "AAD";
            break;
          case ServiceInstanceType::Isolated:
            return "Isolated";
            break;
          case ServiceInstanceType::Shared:
            return "Shared";
            break;
          default:
            throw std::runtime_error("Unknown instance type");
        }
      });

}}}} // namespace Azure::Security::Attestation::Test
