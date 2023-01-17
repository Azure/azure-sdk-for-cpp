// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/identity/managed_identity_credential.hpp>

#include <azure/core/internal/environment.hpp>

#include <chrono>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>

using Azure::Core::_internal::Environment;

namespace {
std::string FormatEnvVarValue(std::string const& varName, bool isSecret)
{
  auto const value = Environment::GetVariable(varName.c_str());
  if (value.empty())
  {
    return varName + " is not defined.";
  }

  if (isSecret)
  {
    return varName + " is " + std::to_string(value.size()) + " characters.";
  }

  return varName + ": " + value;
}

void PrintEnvVariables(std::string const& resourceUsed)
{
  std::cout << std::endl
            << std::endl
            << "Environment:" << std::endl
            << FormatEnvVarValue("MSI_ENDPOINT", false) << std::endl
            << FormatEnvVarValue("MSI_SECRET", true) << std::endl
            << FormatEnvVarValue("IDENTITY_ENDPOINT", false) << std::endl
            << FormatEnvVarValue("IMDS_ENDPOINT", false) << std::endl
            << FormatEnvVarValue("IDENTITY_HEADER", true) << std::endl
            << FormatEnvVarValue("IDENTITY_SERVER_THUMBPRINT", false) << std::endl
            << std::endl
            << FormatEnvVarValue("AZURE_IDENTITY_TEST_MANAGED_IDENTITY_CLIENT_ID", false)
            << std::endl
            << FormatEnvVarValue("AZURE_IDENTITY_TEST_VAULT_URL", false) << resourceUsed
            << std::endl
            << std::endl
            << FormatEnvVarValue("AZURE_LOG_LEVEL", false) << std::endl;
}
} // namespace

int main(int argc, char** argv)
{
  std::string resourceUsedMsg;
  try
  {
    using Azure::DateTime;
    using Azure::Core::Context;
    using Azure::Core::Credentials::TokenCredentialOptions;
    using Azure::Core::Credentials::TokenRequestContext;
    using Azure::Identity::ManagedIdentityCredential;

    constexpr char const* resourceUrlEnvVarName = "AZURE_IDENTITY_TEST_VAULT_URL";
    std::string const defaultResourceUrl = "https://management.azure.com/";

    auto resourceUrl = Environment::GetVariable(resourceUrlEnvVarName);
    if (resourceUrl.empty())
    {
      constexpr char const* simpleSwitch = "--simple";
      if (argc > 0 && std::string(argv[argc - 1]) == simpleSwitch)
      {
        resourceUrl = defaultResourceUrl;
        resourceUsedMsg = " The default '" + defaultResourceUrl + "' was used.";
      }
      else
      {
        std::cout << "FAIL: Test environment is not configured: " << resourceUrlEnvVarName
                  << " is not defined. Either set it, or use '" << simpleSwitch
                  << "' switch. Aborting." << std::endl;
        return -1;
      }
    }

    TokenCredentialOptions options;
    options.Telemetry.ApplicationId = "azure-identity.test.e2e";
    options.Log.AllowedHttpQueryParameters.insert("api-version");
    options.Log.AllowedHttpQueryParameters.insert("clientid");
    options.Log.AllowedHttpQueryParameters.insert("client_id");
    options.Log.AllowedHttpQueryParameters.insert("resource");
    options.Log.AllowedHttpHeaders.insert("Metadata");

    ManagedIdentityCredential credential(
        Environment::GetVariable("AZURE_IDENTITY_TEST_MANAGED_IDENTITY_CLIENT_ID"), options);

    TokenRequestContext tokenRequestContext;
    tokenRequestContext.Scopes = {resourceUrl};

    auto const token = credential.GetToken(tokenRequestContext, Context());

    std::string tokenPreview;
    {
      auto const& tokenStr = token.Token;
      auto const tokenStrLength = tokenStr.length();
      if (tokenStrLength >= 20)
      {
        tokenPreview = "\"" + tokenStr.substr(0, 3) + " ... " + tokenStr.substr(tokenStrLength - 3)
            + "\"" + " (" + std::to_string(tokenStrLength) + " characters).";
      }
      else
      {
        tokenPreview = std::to_string(tokenStrLength) + " characters.";
      }
    }
    std::cout << "OK" << std::endl
              << std::endl
              << std::endl
              << " SSSSSS   UU    UU    CCCCC    CCCCC   EEEEEE   SSSSSS   SSSSSS    (!)\n"
              << " SS       UU    UU   CC       CC       EE       SS       SS        (!)\n"
              << " SSSSSS   UU    UU   CC       CC       EEEEE    SSSSSS   SSSSSS    (!)\n"
              << "     SS   UU    UU   CC       CC       EE           SS       SS       \n"
              << " SSSSSS    UUUUUU     CCCCC    CCCCC   EEEEEE   SSSSSS   SSSSSS    (!)\n"
              << std::endl
              << "Token expiration: " << token.ExpiresOn.ToString(DateTime::DateFormat::Rfc3339)
              << " (" << std::setprecision(2)
              << ((std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::duration<double, DateTime::period>(
                           (token.ExpiresOn - DateTime(std::chrono::system_clock::now()))))
                       .count())
                  / 3600.0)
              << " hours from now)." << std::endl
              << "Token value: " << tokenPreview << std::endl;

    PrintEnvVariables(resourceUsedMsg);
    return 0;
  }
  catch (std::exception const& e)
  {
    std::cout << std::endl
              << std::endl
              << "----------" << std::endl
              << std::endl
              << "ERROR: Exception thrown: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << std::endl
              << std::endl
              << "----------" << std::endl
              << std::endl
              << "ERROR: Unknown exception" << std::endl;
  }

  PrintEnvVariables(resourceUsedMsg);
  return 1;
}
