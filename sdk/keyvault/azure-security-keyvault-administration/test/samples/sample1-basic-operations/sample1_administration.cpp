// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault Settings SDK client for
 * C++ to get one or more settings, and update a setting value.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_HSM_URL:  To the Key Vault HSM URL.
 * - AZURE_TENANT_ID:     Tenant ID for the Azure account.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request.
 * - AZURE_CLIENT_SECRET: The client secret.
 *
 */

#include "get_env.hpp"

#include <azure/identity.hpp>
#include <azure/keyvault/administration.hpp>

#include <chrono>
#include <iostream>

using namespace Azure::Security::KeyVault::Administration;
using namespace std::chrono_literals;

int main()
{
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);

  if (std::getenv("AZURE_KEYVAULT_URL") != std::getenv("AZURE_KEYVAULT_HSM_URL"))
  {

    // create client
    SettingsClient settingsClient(std::getenv("AZURE_KEYVAULT_HSM_URL"), credential);

    try
    {
      // Get all settings
      SettingsListResult settingsList = settingsClient.GetSettings().Value;

      std::cout << "Number of settings found : " << settingsList.Value.size() << std::endl;

      Setting setting = settingsClient.GetSetting(settingsList.Value[0].Name).Value;

      std::cout << "Retrieved setting with name " << setting.Name << ", with value "
                << setting.Value << std::endl;

      UpdateSettingOptions options;
      options.Value = setting.Value;

      Setting updatedSetting
          = settingsClient.UpdateSetting(settingsList.Value[0].Name, options).Value;

      std::cout << "Retrieved updated setting with name " << updatedSetting.Name << ", with value "
                << updatedSetting.Value << std::endl;
    }
    catch (Azure::Core::Credentials::AuthenticationException const& e)
    {
      std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
      return 1;
    }
    catch (Azure::Core::RequestFailedException const& e)
    {
      std::cout << "Key Vault Settings Client Exception happened:" << std::endl
                << e.Message << std::endl;
      return 1;
    }
  }
  else
  {
    std::cout << "this sample requires an HSM to be present and configured " << std::endl;
  }

  return 0;
}
