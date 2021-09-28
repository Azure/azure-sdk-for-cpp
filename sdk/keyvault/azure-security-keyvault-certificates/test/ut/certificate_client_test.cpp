// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/identity/client_secret_credential.hpp>

#include "certificate_client_base_test.hpp"
#include <cstddef>
#include <gtest/gtest.h>

#include <string>

using namespace std::chrono_literals;
using namespace Azure::Security::KeyVault::Certificates;
using namespace Azure::Security::KeyVault::Certificates::Test;

using namespace std::chrono_literals;

// NOTE:
// Disabling test as the createCertificate operation is currently broken. See:
// https://github.com/Azure/azure-sdk-for-cpp/issues/2938

TEST_F(KeyVaultCertificateClientTest, DISABLED_CreateCertificate)
{
  // cspell: disable-next-line
  std::string const certificateName("magiqStuff289123");

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  auto params = CertificateCreateParameters();
  params.Policy.Subject = "CN=xyz";
  params.Policy.ValidityInMonths = 12;
  params.Policy.Enabled = true;

  params.Properties.Enabled = true;
  params.Properties.Name = certificateName;
  params.Policy.ContentType = CertificateContentType::Pkcs12;
  params.Policy.IssuerName = "Self";

  LifetimeAction action;
  action.LifetimePercentage = 80;
  action.Action = CertificatePolicyAction::AutoRenew;
  params.Policy.LifetimeActions.emplace_back(action);

  auto response = client.StartCreateCertificate(certificateName, params);
  auto result = response.PollUntilDone(m_defaultWait);

  EXPECT_EQ(result.Value.Name(), params.Properties.Name);
  EXPECT_EQ(result.Value.Properties.Enabled.Value(), true);
}

TEST_F(KeyVaultCertificateClientTest, DISABLED_GetCertificate)
{
  // cspell: disable-next-line
  std::string const certificateName("vivazqu");

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  auto params = CertificateCreateParameters();
  params.Policy.Subject = "CN=xyz";
  params.Policy.ValidityInMonths = 12;
  params.Policy.Enabled = true;

  params.Properties.Enabled = true;
  params.Properties.Name = certificateName;
  params.Policy.ContentType = CertificateContentType::Pkcs12;
  params.Policy.IssuerName = "Self";

  LifetimeAction action;
  action.LifetimePercentage = 80;
  action.Action = CertificatePolicyAction::AutoRenew;
  params.Policy.LifetimeActions.emplace_back(action);

  {
    auto response = client.StartCreateCertificate(certificateName, params);
    auto result = response.PollUntilDone(m_defaultWait);
  }

  {
    auto response = client.GetCertificate(certificateName);
    CheckValidResponse(response);
    auto cert = response.Value;
    EXPECT_EQ(cert.Name(), cert.Properties.Name);
    EXPECT_EQ(cert.Properties.Name, certificateName);
    EXPECT_EQ(cert.Properties.VaultUrl, m_keyVaultUrl);
    // There should be a version
    EXPECT_NE(cert.Properties.Version, "");

    // x5t
    EXPECT_NE(cert.Properties.X509Thumbprint.size(), 0);
    EXPECT_EQ(cert.Properties.Tags.size(), 0);

    // attributes
    EXPECT_TRUE(cert.Properties.Enabled.HasValue());
    EXPECT_TRUE(cert.Properties.NotBefore.HasValue());
    EXPECT_TRUE(cert.Properties.ExpiresOn.HasValue());
    EXPECT_TRUE(cert.Properties.CreatedOn.HasValue());
    EXPECT_TRUE(cert.Properties.UpdatedOn.HasValue());
    EXPECT_TRUE(cert.Properties.RecoverableDays.HasValue());
    EXPECT_TRUE(cert.Properties.RecoveryLevel.HasValue());

    // kid, sid, cer
    EXPECT_NE(cert.KeyId, "");
    EXPECT_NE(cert.SecretId, "");
    EXPECT_NE(cert.Cer.size(), 0);

    // policy
    {
      auto const& policy = cert.Policy;

      // Key props
      EXPECT_TRUE(policy.Exportable.HasValue());
      EXPECT_TRUE(policy.KeyType.HasValue());
      EXPECT_TRUE(policy.ReuseKey.HasValue());
      // Recording uses RSA with no curve-name. Use RSA key when running LIVE
      EXPECT_FALSE(policy.KeyCurveName.HasValue());
      EXPECT_TRUE(policy.KeySize.HasValue());

      // Secret props
      EXPECT_TRUE(policy.ContentType.HasValue());

      // x509_props
      EXPECT_TRUE(policy.Subject.size() > 0);

      // issuer
      EXPECT_TRUE(policy.IssuerName.HasValue());

      // attributes
      EXPECT_TRUE(policy.CreatedOn.HasValue());

      // lifetime_actions
      EXPECT_TRUE(policy.LifetimeActions.size() > 0);
      EXPECT_NE(policy.LifetimeActions[0].Action.ToString(), "");
    }
  }
}

TEST_F(KeyVaultCertificateClientTest, DISABLED_GetCertificateVersion)
{
  // cspell: disable-next-line
  std::string const certificateName("vivazqu2");

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());
  auto params = CertificateCreateParameters();
  params.Policy.Subject = "CN=xyz";
  params.Policy.ValidityInMonths = 12;
  params.Policy.Enabled = true;

  params.Properties.Enabled = true;
  params.Properties.Name = certificateName;
  params.Policy.ContentType = CertificateContentType::Pkcs12;
  params.Policy.IssuerName = "Self";

  LifetimeAction action;
  action.LifetimePercentage = 80;
  action.Action = CertificatePolicyAction::AutoRenew;
  params.Policy.LifetimeActions.emplace_back(action);
  GetCertificateOptions options;
  {
    auto response = client.StartCreateCertificate(certificateName, params);
    auto result = response.PollUntilDone(m_defaultWait);
    options.Version = result.Value.Properties.Version;
  }
  {

    auto response = client.GetCertificateVersion(certificateName, options);
    CheckValidResponse(response);
    auto cert = response.Value;
    EXPECT_EQ(cert.Name(), cert.Properties.Name);
    EXPECT_EQ(cert.Properties.Name, certificateName);
    EXPECT_EQ(cert.Properties.VaultUrl, m_keyVaultUrl);
    // There should be a version
    EXPECT_NE(cert.Properties.Version, "");

    // x5t
    EXPECT_NE(cert.Properties.X509Thumbprint.size(), 0);
    EXPECT_EQ(cert.Properties.Tags.size(), 0);

    // attributes
    EXPECT_TRUE(cert.Properties.Enabled.HasValue());
    EXPECT_TRUE(cert.Properties.NotBefore.HasValue());
    EXPECT_TRUE(cert.Properties.ExpiresOn.HasValue());
    EXPECT_TRUE(cert.Properties.CreatedOn.HasValue());
    EXPECT_TRUE(cert.Properties.UpdatedOn.HasValue());
    EXPECT_TRUE(cert.Properties.RecoverableDays.HasValue());
    EXPECT_TRUE(cert.Properties.RecoveryLevel.HasValue());

    // kid, sid, cer
    EXPECT_NE(cert.KeyId, "");
    EXPECT_NE(cert.SecretId, "");
    EXPECT_NE(cert.Cer.size(), 0);
  }
}
