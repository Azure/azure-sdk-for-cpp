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

  class CertificateTests : public Azure::Core::Test::TestBase {
  private:
  protected:
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;

    // Create
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
    }

    std::string GetServiceEndpoint(ServiceInstanceType const type)
    {
      if (type == ServiceInstanceType::Shared)
      {
        std::string const shortLocation(GetEnv("LOCATION_SHORT_NAME"));
        return "https://shared" + shortLocation + "." + shortLocation + ".attest.azure.net";
      }
      else if (type == ServiceInstanceType::AAD)
      {
        return GetEnv("ATTESTATION_AAD_URL");
      }
      else if (type == ServiceInstanceType::Isolated)
      {
        return GetEnv("ATTESTATION_ISOLATED_URL");
      }
      throw std::runtime_error("Invalid instance type.");
    }

    Azure::Security::Attestation::AttestationTokenValidationOptions GetTokenValidationOptions()
    {
      AttestationTokenValidationOptions returnValue{};

      if (m_testContext.IsPlaybackMode())
      {
        // Skip validating time stamps if using recordings.
        returnValue.ValidateNotBeforeTime = false;
        returnValue.ValidateExpirationTime = false;
      }
      else
      {
        returnValue.TimeValidationSlack = 10s;
      }
      return returnValue;
    }

    AttestationAdministrationClient CreateClient(ServiceInstanceType instanceType)
    {
      // `InitTestClient` takes care of setting up Record&Playback.
      AttestationAdministrationClientOptions options
          = InitClientOptions<AttestationAdministrationClientOptions>();
      options.TokenValidationOptions = GetTokenValidationOptions();

      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
          = CreateClientSecretCredential(
              GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

      return AttestationAdministrationClient::Create(
          GetServiceEndpoint(instanceType), credential, options);
    }

    // Get Policy management certificates for each instance type.
    // The GetIsolatedModeManagementCertificates API can be run against all instance types, but it
    // only returns values on isolated instances (an isolated instance is defined to be an
    // attestation service instance with policy management certificates).
    void GetIsolatedModeCertificatesTest(ServiceInstanceType const instanceType)
    {
      auto adminClient(CreateClient(instanceType));

      {
        auto certificatesResult = adminClient.GetIsolatedModeCertificates(
            GetIsolatedModeCertificatesOptions{GetTokenValidationOptions()});

        // Do we expect to get any certificates in the response? AAD and Shared instances will never
        // have any certificates.
        bool expectedCertificates = false;
        if (instanceType == ServiceInstanceType::Isolated)
        {
          expectedCertificates = true;
        }

        if (expectedCertificates)
        {
          ASSERT_NE(0ul, certificatesResult.Value.Body.Certificates.size());
        }
        else
        {
          ASSERT_EQ(0ul, certificatesResult.Value.Body.Certificates.size());
        }

        // In playback mode, the endpoint is a mocked value so the Issuer in the result will not
        // match.
        if (!m_testContext.IsPlaybackMode())
        {
          EXPECT_EQ(GetServiceEndpoint(instanceType), *certificatesResult.Value.Issuer);

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

      // Per-test-suite set-up.
    // Called before the first test in this test suite.
    // Can be omitted if not needed.
    static void SetUpTestSuite()
    {
      _putenv_s("AZURE_TEST_MODE", "RECORD");
      std::system("pwsh Set-ExecutionPolicy -Scope CurrentUser Unrestricted");
      auto result = std::system("pwsh "
                  "testproxy.ps1");
      std::cout << result;
    }

    // Per-test-suite tear-down.
    // Called after the last test in this test suite.
    // Can be omitted if not needed.
    static void TearDownTestSuite()
    {
      std::system("pwsh "
                  "stopProxy.ps1");
    }
  }; // namespace Test

  // Get Policy management certificates for each instance type.
  // The GetIsolatedModeManagementCertificates API can be run against all instance types, but it
  // only returns values on isolated instances (an isolated instance is defined to be an attestation
  // service instance with policy management certificates).
  TEST_F(CertificateTests, GetPolicyManagementCertificatesAad)
  {
    GetIsolatedModeCertificatesTest(ServiceInstanceType::AAD);
  }
  TEST_F(CertificateTests, GetPolicyManagementCertificatesIsolated)
  {
    GetIsolatedModeCertificatesTest(ServiceInstanceType::Isolated);
  }
  TEST_F(CertificateTests, GetPolicyManagementCertificatesShared)
  {
    GetIsolatedModeCertificatesTest(ServiceInstanceType::Shared);
  }

  TEST_F(CertificateTests, AddPolicyManagementCertificate_LIVEONLY_)
  {
    CHECK_SKIP_TEST()

    auto adminClient(CreateClient(ServiceInstanceType::Isolated));

    auto isolatedCertificateBase64(GetEnv("ISOLATED_SIGNING_CERTIFICATE"));
    auto isolatedCertificate(Cryptography::ImportX509Certificate(
        Cryptography::PemFromBase64(isolatedCertificateBase64, "CERTIFICATE")));

    // Load the preconfigured policy certificate to add.
    auto certificateToAddBase64(GetEnv("POLICY_SIGNING_CERTIFICATE_0"));
    auto certificateToAdd(Cryptography::ImportX509Certificate(
        Cryptography::PemFromBase64(certificateToAddBase64, "CERTIFICATE")));
    std::string expectedThumbprint = certificateToAdd->GetThumbprint();

    {
      auto isolatedKeyBase64(GetEnv("ISOLATED_SIGNING_KEY"));
      std::unique_ptr<Cryptography::AsymmetricKey> isolatedPrivateKey(
          Cryptography::ImportPrivateKey(
              Cryptography::PemFromBase64(isolatedKeyBase64, "PRIVATE KEY")));

      // Create a signing key to be used when signing the request to the service.
      auto isolatedSigningKey(AttestationSigningKey{
          isolatedPrivateKey->ExportPrivateKey(), isolatedCertificate->ExportAsPEM()});

      auto certificatesResult = adminClient.AddIsolatedModeCertificate(
          certificateToAdd->ExportAsPEM(), isolatedSigningKey);

      EXPECT_EQ(
          Models::PolicyCertificateModification::IsPresent,
          certificatesResult.Value.Body.CertificateModification);

      // And the thumbprint indicates which certificate was added.
      EXPECT_EQ(expectedThumbprint, certificatesResult.Value.Body.CertificateThumbprint);
    }

    // Make sure that the certificate we just added is included in the enumeration.
    {
      auto policyCertificates = adminClient.GetIsolatedModeCertificates();
      EXPECT_GT(policyCertificates.Value.Body.Certificates.size(), 1ul);

      bool foundIsolatedCertificate = false;
      bool foundAddedCertificate = false;
      for (const auto& signer : policyCertificates.Value.Body.Certificates)
      {
        auto signerCertificate
            = Cryptography::ImportX509Certificate(((*signer.CertificateChain)[0]));
        if (signerCertificate->GetThumbprint() == isolatedCertificate->GetThumbprint())
        {
          foundIsolatedCertificate = true;
        }
        if (signerCertificate->GetThumbprint() == expectedThumbprint)
        {
          foundAddedCertificate = true;
        }
      }
      EXPECT_TRUE(foundIsolatedCertificate);
      EXPECT_TRUE(foundAddedCertificate);
    }
  }

  TEST_F(CertificateTests, RemovePolicyManagementCertificate_LIVEONLY_)
  {
    CHECK_SKIP_TEST()

    auto adminClient(CreateClient(ServiceInstanceType::Isolated));

    auto isolatedCertificateBase64(GetEnv("ISOLATED_SIGNING_CERTIFICATE"));
    auto isolatedCertificate(Cryptography::ImportX509Certificate(
        Cryptography::PemFromBase64(isolatedCertificateBase64, "CERTIFICATE")));

    // Load the preconfigured policy certificate to add.
    auto certificateToRemoveBase64(GetEnv("POLICY_SIGNING_CERTIFICATE_0"));
    auto certificateToRemove(Cryptography::ImportX509Certificate(
        Cryptography::PemFromBase64(certificateToRemoveBase64, "CERTIFICATE")));
    std::string expectedThumbprint = certificateToRemove->GetThumbprint();

    // Create a signing key to be used when signing the request to the service. We use the ISOLATED
    // SIGNING KEY because we know that it will always be present.
    auto isolatedKeyBase64(GetEnv("ISOLATED_SIGNING_KEY"));
    std::unique_ptr<Cryptography::AsymmetricKey> isolatedPrivateKey(Cryptography::ImportPrivateKey(
        Cryptography::PemFromBase64(isolatedKeyBase64, "PRIVATE KEY")));

    auto isolatedSigningKey(AttestationSigningKey{
        isolatedPrivateKey->ExportPrivateKey(), isolatedCertificate->ExportAsPEM()});

    // Ensure that POLICY_SIGNING_CERTIFICATE_0 is already present in the list of certificates.
    {
      auto certificatesResult = adminClient.AddIsolatedModeCertificate(
          certificateToRemove->ExportAsPEM(), isolatedSigningKey);

      EXPECT_EQ(
          Models::PolicyCertificateModification::IsPresent,
          certificatesResult.Value.Body.CertificateModification);
    }

    // And now remove that certificate.
    {
      auto certificatesResult = adminClient.RemoveIsolatedModeCertificate(
          certificateToRemove->ExportAsPEM(), isolatedSigningKey);

      EXPECT_EQ(
          Models::PolicyCertificateModification::IsAbsent,
          certificatesResult.Value.Body.CertificateModification);

      // And the thumbprint indicates which certificate was removed.
      EXPECT_EQ(expectedThumbprint, certificatesResult.Value.Body.CertificateThumbprint);
    }

    // Make sure that the certificate we just removed is NOT included in the enumeration.
    {
      auto policyCertificates = adminClient.GetIsolatedModeCertificates();
      EXPECT_EQ(policyCertificates.Value.Body.Certificates.size(), 1ul);

      bool foundIsolatedCertificate = false;
      bool foundAddedCertificate = false;
      for (const auto& signer : policyCertificates.Value.Body.Certificates)
      {
        auto signerCertificate
            = Cryptography::ImportX509Certificate(((*signer.CertificateChain)[0]));
        if (signerCertificate->GetThumbprint() == isolatedCertificate->GetThumbprint())
        {
          foundIsolatedCertificate = true;
        }
        if (signerCertificate->GetThumbprint() == expectedThumbprint)
        {
          foundAddedCertificate = true;
        }
      }
      EXPECT_TRUE(foundIsolatedCertificate);
      EXPECT_FALSE(foundAddedCertificate);
    }
  }

  // Verify that we get an exception if we try to set a policy management certificate on an AAD
  // instance. THe primary purpose of this test is to increase code coverage numbers.
  TEST_F(CertificateTests, VerifyFailedAddCertificate)
  {
    auto adminClient(CreateClient(ServiceInstanceType::AAD));

    // Create a signing key to be used when signing the request to the service. We use the ISOLATED
    // SIGNING KEY because we know that it will always be present.
    auto fakedIsolatedKey(Cryptography::CreateRsaKey(2048));
    auto fakedIsolatedCertificate(
        Cryptography::CreateX509CertificateForPrivateKey(fakedIsolatedKey, "CN=FakeIsolatedKey"));

    // Load the preconfigured policy certificate to add.
    auto keyToAdd(Cryptography::CreateRsaKey(2048));
    auto fakedCertificateToAdd(
        Cryptography::CreateX509CertificateForPrivateKey(keyToAdd, "CN=FakeIsolatedKey"));

    auto isolatedSigningKey(AttestationSigningKey{
        fakedIsolatedKey->ExportPrivateKey(), fakedIsolatedCertificate->ExportAsPEM()});

    {
      EXPECT_THROW(
          adminClient.AddIsolatedModeCertificate(
              fakedCertificateToAdd->ExportAsPEM(), isolatedSigningKey),
          Azure::Core::RequestFailedException);
    }
  }
  // Verify that we get an exception if we try to remove a policy management certificate on an AAD
  // instance. THe primary purpose of this test is to increase code coverage numbers.
  TEST_F(CertificateTests, VerifyFailedRemoveCertificate)
  {
    auto adminClient(CreateClient(ServiceInstanceType::AAD));

    // Create a signing key to be used when signing the request to the service. We use the ISOLATED
    // SIGNING KEY because we know that it will always be present.
    auto fakedIsolatedKey(Cryptography::CreateRsaKey(2048));
    auto fakedIsolatedCertificate(
        Cryptography::CreateX509CertificateForPrivateKey(fakedIsolatedKey, "CN=FakeIsolatedKey"));

    // Load the preconfigured policy certificate to add.
    auto keyToAdd(Cryptography::CreateRsaKey(2048));
    auto fakedCertificateToRemove(
        Cryptography::CreateX509CertificateForPrivateKey(keyToAdd, "CN=FakeIsolatedKey"));

    auto isolatedSigningKey(AttestationSigningKey{
        fakedIsolatedKey->ExportPrivateKey(), fakedIsolatedCertificate->ExportAsPEM()});

    {
      EXPECT_THROW(
          adminClient.RemoveIsolatedModeCertificate(
              fakedCertificateToRemove->ExportAsPEM(), isolatedSigningKey),
          Azure::Core::RequestFailedException);
    }
  }

}}}} // namespace Azure::Security::Attestation::Test
