// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample demonstrates using the Attestation Service SDK for C++ to add (and remove) a
 * new policy management certificate from an Isolated mode attestation service instance.
 *
 * Remember that when an attestation service instance is in isolated mode, the service is configured
 * with a set of X.509 certificates. This sample adds a new certificate to the existing set of
 * certificates.
 *
 * @remark The following environment variables must be set before running the sample.
 * - ATTESTATION_ISOLATED_URL:  Points to an Attestation Service Instance in Isolated mode.
 * operations.
 * - ISOLATED_SIGNING_KEY: The private key which was used when creating the isolated attestation
 * instance.
 * - ISOLATED_SIGNING_CERTIFICATE: An X.509 certificate which wraps the ISOLATED_SIGNING_KEY.
 * - POLICY_SIGNING_CERTIFICATE_0: An X.509 certificate which will be added to the set of policy
 * management certificates.
 * - AZURE_TENANT_ID:     Tenant ID for the Azure account.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request.
 * - AZURE_CLIENT_SECRET: The client secret.
 *
 */

#include <get_env.hpp>

#include "cryptohelpers.hpp"
#include <azure/attestation.hpp>
#include <azure/identity.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

// cspell:: words mrsigner mrenclave mitm
using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace std::chrono_literals;
using namespace Azure::Core;

int main()
{
  try
  {
    // create an administration client
    auto const credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
        GetEnvHelper::GetEnv("AZURE_TENANT_ID"),
        GetEnvHelper::GetEnv("AZURE_CLIENT_ID"),
        GetEnvHelper::GetEnv("AZURE_CLIENT_SECRET"));
    std::shared_ptr<AttestationAdministrationClient> adminClient(
        AttestationAdministrationClient::CreatePointer(
            GetEnvHelper::GetEnv("ATTESTATION_ISOLATED_URL"), credential));

    std::string const signingKey(GetEnvHelper::GetEnv("ISOLATED_SIGNING_KEY"));
    std::string const signingCert(GetEnvHelper::GetEnv("ISOLATED_SIGNING_CERTIFICATE"));

    // The attestation APIs expect a PEM encoded key and certificate, so convert the Base64 key and
    // certificate to PEM encoded equivalents.
    std::string const pemSigningKey(::Cryptography::PemFromBase64(signingKey, "PRIVATE KEY"));
    std::string const pemSigningCert(::Cryptography::PemFromBase64(signingCert, "CERTIFICATE"));

    AttestationSigningKey const requestSigner{pemSigningKey, pemSigningCert};

    // We start this sample by adding a new certificate to the set of policy management
    // certificates.
    {
      // Create a PEM encoded X.509 certificate to add based on the POLICY_SIGNING_CERTIFICATE_0
      // certificate.
      std::string const certToAdd(GetEnvHelper::GetEnv("POLICY_SIGNING_CERTIFICATE_0"));
      std::string const pemCertificateToAdd(
          ::Cryptography::PemFromBase64(certToAdd, "CERTIFICATE"));

      // Add the new certificate to the set of policy management certificates for this attestation
      // service instance.
      Azure::Response<AttestationToken<IsolatedModeCertificateModificationResult>> const addResult
          = adminClient->AddIsolatedModeCertificate(pemCertificateToAdd, requestSigner);

      std::cout << "The result of the certificate add operation is: "
                << addResult.Value.Body.CertificateModification.ToString() << std::endl;

      if (addResult.Value.Body.CertificateModification != PolicyCertificateModification::IsPresent)
      {
        throw std::runtime_error("After adding certificate, it is not present :(.");
      }

      std::cout << "The thumbprint of the certificate from the result is: "
                << addResult.Value.Body.CertificateThumbprint << std::endl;

      auto const x509Cert(::Cryptography::ImportX509Certificate(pemCertificateToAdd));
      std::cout << "The thumbprint of the certificate to be added is: " << x509Cert->GetThumbprint()
                << std::endl;

      if (x509Cert->GetThumbprint() != addResult.Value.Body.CertificateThumbprint)
      {
        throw std::runtime_error(
            "Certificate added was not the requested certificate to be added.");
      }
    }

    //
    //  And now remove the certificate we just added.
    //
    // We start this sample by adding a new certificate to the set of policy management
    // certificates.
    {
      // Create a PEM encoded X.509 certificate to add based on the POLICY_SIGNING_CERTIFICATE_0
      // certificate.
      std::string const certToRemove(GetEnvHelper::GetEnv("POLICY_SIGNING_CERTIFICATE_0"));
      std::string const pemCertificateToRemove(
          ::Cryptography::PemFromBase64(certToRemove, "CERTIFICATE"));

      // Add the new certificate to the set of policy management certificates for this attestation
      // service instance.
      Azure::Response<AttestationToken<IsolatedModeCertificateModificationResult>> const addResult
          = adminClient->RemoveIsolatedModeCertificate(pemCertificateToRemove, requestSigner);

      std::cout << "The result of the certificate remove operation is: "
                << addResult.Value.Body.CertificateModification.ToString() << std::endl;

      if (addResult.Value.Body.CertificateModification != PolicyCertificateModification::IsAbsent)
      {
        throw std::runtime_error("After adding certificate, it is not present :(.");
      }

      std::cout << "The thumbprint of the certificate from the result is: "
                << addResult.Value.Body.CertificateThumbprint << std::endl;

      auto const x509Cert(::Cryptography::ImportX509Certificate(pemCertificateToRemove));
      std::cout << "The thumbprint of the certificate to be removed is: "
                << x509Cert->GetThumbprint() << std::endl;

      if (x509Cert->GetThumbprint() != addResult.Value.Body.CertificateThumbprint)
      {
        throw std::runtime_error(
            "Certificate removed was not the requested certificate to be removed.");
      }
    }
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Request Failed Exception happened:" << std::endl << e.what() << std::endl;
    if (e.RawResponse)
    {
      std::cout << "Error Code: " << e.ErrorCode << std::endl;
      std::cout << "Error Message: " << e.Message << std::endl;
    }
    return 1;
  }
  return 0;
}
