﻿// Copyright (c) Microsoft Corporation. All rights reserved.
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

TEST_F(KeyVaultCertificateClientTest, CreateCertificate)
{
  // cspell: disable-next-line
  std::string const certificateName("magiqStuff");

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

TEST_F(KeyVaultCertificateClientTest, GetCertificate)
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

TEST_F(KeyVaultCertificateClientTest, GetCertificateVersion)
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

TEST_F(KeyVaultCertificateClientTest, CreateGetIssuer)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  CertificateIssuer issuer;
  issuer.Name = "issuer01";
  issuer.Provider = "Test";
  issuer.Properties.Enabled = true;
  issuer.Credentials.AccountId = "keyvaultuser";
  issuer.Credentials.Password = "password";

  AdministratorDetails admin;
  admin.FirstName = "John";
  admin.LastName = "Doe";
  admin.EmailAddress = "admin@microsoft.com";
  admin.PhoneNumber = "4255555555";

  issuer.Organization.AdminDetails.emplace_back(admin);

  {
    auto result = client.CreateIssuer(issuer);
    CheckIssuers(result.Value, issuer);
  }

  {
    auto result = client.GetIssuer(issuer.Name);
    CheckIssuers(result.Value, issuer);
  }

  {
    auto result = client.DeleteIssuer(issuer.Name);
    CheckIssuers(result.Value, issuer);
  }
}

TEST_F(KeyVaultCertificateClientTest, UpdateIssuer)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  CertificateIssuer issuer;
  issuer.Name = "issuer01";
  issuer.Provider = "Test";
  issuer.Properties.Enabled = true;
  issuer.Credentials.AccountId = "keyvaultuser";
  issuer.Credentials.Password = "password";

  AdministratorDetails admin;
  admin.FirstName = "John";
  admin.LastName = "Doe";
  admin.EmailAddress = "admin@microsoft.com";
  admin.PhoneNumber = "4255555555";

  issuer.Organization.AdminDetails.emplace_back(admin);

  {
    auto result = client.CreateIssuer(issuer);
    CheckIssuers(result.Value, issuer);
  }

  {
    issuer.Credentials.Password = "password2";
    auto result = client.UpdateIssuer(issuer);
    CheckIssuers(result.Value, issuer);
  }

  {
    auto result = client.DeleteIssuer(issuer.Name);
    CheckIssuers(result.Value, issuer);
  }
}

TEST_F(KeyVaultCertificateClientTest, SetContacts)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  std::vector<CertificateContact> contacts;

  CertificateContact ctt;

  ctt.EmailAddress = "one@two.org";
  ctt.Name = "giqu"; // cspell:disable-line
  ctt.Phone = "1234567890";
  contacts.emplace_back(ctt);

  CertificateContact ctt2;

  ctt2.EmailAddress = "two@three.org";
  ctt2.Name = "giqu2"; // cspell:disable-line
  ctt2.Phone = "1234567891";
  contacts.emplace_back(ctt2);

  auto response = client.SetContacts(contacts);

  CheckContactsCollections(contacts, response.Value);

  auto response2 = client.DeleteContacts();

  CheckContactsCollections(contacts, response2.Value);
}

TEST_F(KeyVaultCertificateClientTest, GetContacts)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  std::vector<CertificateContact> contacts;

  CertificateContact ctt;

  ctt.EmailAddress = "one@two.org";
  ctt.Name = "giqu"; // cspell:disable-line
  ctt.Phone = "1234567890";
  contacts.emplace_back(ctt);

  CertificateContact ctt2;

  ctt2.EmailAddress = "two@three.org";
  ctt2.Name = "giqu2"; // cspell:disable-line
  ctt2.Phone = "1234567891";
  contacts.emplace_back(ctt2);

  client.SetContacts(contacts);
  auto response = client.GetContacts();

  CheckContactsCollections(contacts, response.Value);

  auto response2 = client.DeleteContacts();

  CheckContactsCollections(contacts, response2.Value);
}

TEST_F(KeyVaultCertificateClientTest, GetContactsPartial)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  std::vector<CertificateContact> contacts;

  CertificateContact ctt;

  ctt.EmailAddress = "one1@two.org";
  contacts.emplace_back(ctt);

  CertificateContact ctt2, ctt3;

  ctt2.EmailAddress = "two2@three.org";
  ctt2.Name = "giqu2"; // cspell:disable-line
  contacts.emplace_back(ctt2);

  ctt3.EmailAddress = "two3@three.org";
  ctt3.Phone = "1234567891";

  contacts.emplace_back(ctt3);

  client.SetContacts(contacts);
  auto response = client.GetContacts();

  CheckContactsCollections(contacts, response.Value);

  auto response2 = client.DeleteContacts();

  CheckContactsCollections(contacts, response2.Value);
}

TEST_F(KeyVaultCertificateClientTest, GetContactsDuplicateEmail)
{
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  std::vector<CertificateContact> contacts;

  CertificateContact ctt;

  ctt.EmailAddress = "one1@two.org";
  contacts.emplace_back(ctt);

  CertificateContact ctt2, ctt3;

  ctt2.EmailAddress = "two@three.org";
  ctt2.Name = "giqu2"; // cspell:disable-line
  contacts.emplace_back(ctt2);

  ctt3.EmailAddress = "two@three.org";
  ctt3.Phone = "1234567891";

  contacts.emplace_back(ctt3);

  client.SetContacts(contacts);
  auto response = client.GetContacts();

  CheckContactsCollections(contacts, response.Value);

  auto response2 = client.DeleteContacts();

  CheckContactsCollections(contacts, response2.Value);
}

TEST_F(KeyVaultCertificateClientTest, GetCertificatePolicy)
{
  // cspell: disable-next-line
  std::string const certificateName("certPolicy");

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
    response.PollUntilDone(m_defaultWait);
  }

  {
    auto response = client.GetCertificatePolicy(certificateName);
    auto const& policy = response.Value;

    // Key props
    EXPECT_TRUE(policy.Exportable.HasValue());
    EXPECT_TRUE(policy.KeyType.HasValue());
    EXPECT_TRUE(policy.ReuseKey.HasValue());
    // Recording uses RSA with no curve-name. Use RSA key when running LIVE
    EXPECT_FALSE(policy.KeyCurveName.HasValue());
    EXPECT_TRUE(policy.KeySize.HasValue());
    // enabled
    EXPECT_TRUE(policy.Enabled.HasValue());
    EXPECT_TRUE(policy.Enabled.Value());
    // validity
    EXPECT_TRUE(policy.ValidityInMonths.HasValue());
    EXPECT_EQ(policy.ValidityInMonths.Value(), 12);
    // Secret props
    EXPECT_TRUE(policy.ContentType.HasValue());
    EXPECT_EQ(policy.ContentType.Value(), CertificateContentType::Pkcs12);
    // x509_props
    EXPECT_TRUE(policy.Subject.size() > 0);
    EXPECT_EQ(policy.Subject, "CN=xyz");
    // issuer
    EXPECT_TRUE(policy.IssuerName.HasValue());
    EXPECT_EQ(policy.IssuerName.Value(), "Self");
    // attributes
    EXPECT_TRUE(policy.CreatedOn.HasValue());
    // lifetime_actions
    EXPECT_TRUE(policy.LifetimeActions.size() > 0);
    EXPECT_NE(policy.LifetimeActions[0].Action.ToString(), "");
  }
}

TEST_F(KeyVaultCertificateClientTest, UpdateCertificatePolicy)
{
  // cspell: disable-next-line
  std::string const certificateName("updateCertPolicy");

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
    response.PollUntilDone(m_defaultWait);
  }

  {
    auto response = client.GetCertificatePolicy(certificateName);
    auto policy = response.Value;

    // Key props
    EXPECT_TRUE(policy.Exportable.HasValue());
    EXPECT_TRUE(policy.KeyType.HasValue());
    EXPECT_TRUE(policy.ReuseKey.HasValue());
    // Recording uses RSA with no curve-name. Use RSA key when running LIVE
    EXPECT_FALSE(policy.KeyCurveName.HasValue());
    EXPECT_TRUE(policy.KeySize.HasValue());
    // enabled
    EXPECT_TRUE(policy.Enabled.HasValue());
    EXPECT_TRUE(policy.Enabled.Value());
    // validity
    EXPECT_TRUE(policy.ValidityInMonths.HasValue());
    EXPECT_EQ(policy.ValidityInMonths.Value(), 12);
    // Secret props
    EXPECT_TRUE(policy.ContentType.HasValue());
    EXPECT_EQ(policy.ContentType.Value(), CertificateContentType::Pkcs12);
    // x509_props
    EXPECT_TRUE(policy.Subject.size() > 0);
    EXPECT_EQ(policy.Subject, "CN=xyz");
    // issuer
    EXPECT_TRUE(policy.IssuerName.HasValue());
    EXPECT_EQ(policy.IssuerName.Value(), "Self");
    // attributes
    EXPECT_TRUE(policy.CreatedOn.HasValue());
    // lifetime_actions
    EXPECT_TRUE(policy.LifetimeActions.size() > 0);
    EXPECT_NE(policy.LifetimeActions[0].Action.ToString(), "");

    policy.ValidityInMonths = 8;
    policy.Subject = "CN=twa";

    auto updateResponse = client.UpdateCertificatePolicy(certificateName, policy);
    auto const& updatedPolicy = updateResponse.Value;

    // Key props
    EXPECT_TRUE(updatedPolicy.Exportable.HasValue());
    EXPECT_TRUE(updatedPolicy.KeyType.HasValue());
    EXPECT_TRUE(updatedPolicy.ReuseKey.HasValue());
    // Recording uses RSA with no curve-name. Use RSA key when running LIVE
    EXPECT_FALSE(updatedPolicy.KeyCurveName.HasValue());
    EXPECT_TRUE(updatedPolicy.KeySize.HasValue());
    // enabled
    EXPECT_TRUE(updatedPolicy.Enabled.HasValue());
    EXPECT_TRUE(updatedPolicy.Enabled.Value());
    // validity
    EXPECT_TRUE(updatedPolicy.ValidityInMonths.HasValue());
    EXPECT_EQ(updatedPolicy.ValidityInMonths.Value(), 8);
    // Secret props
    EXPECT_TRUE(updatedPolicy.ContentType.HasValue());
    EXPECT_EQ(updatedPolicy.ContentType.Value(), CertificateContentType::Pkcs12);
    // x509_props
    EXPECT_TRUE(updatedPolicy.Subject.size() > 0);
    EXPECT_EQ(updatedPolicy.Subject, "CN=twa");
    // issuer
    EXPECT_TRUE(updatedPolicy.IssuerName.HasValue());
    EXPECT_EQ(updatedPolicy.IssuerName.Value(), "Self");
    // attributes
    EXPECT_TRUE(updatedPolicy.CreatedOn.HasValue());
    // lifetime_actions
    EXPECT_TRUE(updatedPolicy.LifetimeActions.size() > 0);
    EXPECT_NE(updatedPolicy.LifetimeActions[0].Action.ToString(), "");
  }
}
