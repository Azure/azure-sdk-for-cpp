// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample provides the code implementation to use the Key Vault Settings SDK client for
 * C++ to get one or more settings, and update a setting value.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_HSM_URL:  To the Key Vault HSM URL.
 *
 */

#include <azure/identity.hpp>
#include <azure/keyvault/administration.hpp>

#include <chrono>
#include <iostream>

using namespace Azure::Security::KeyVault::Administration;
using namespace Azure::Security::KeyVault::Administration::Models;
using namespace std::chrono_literals;

int main()
{
  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();

  // create client
  SettingsClient settingsClient(
      Azure::Core::_internal::Environment::GetVariable("AZURE_KEYVAULT_HSM_URL"), credential);

  try
  {
    // Get all settings
    SettingsListResult settingsList = settingsClient.GetSettings().Value;

    std::cout << "Number of settings found : " << settingsList.Value.size() << std::endl;

    Setting setting = settingsClient.GetSetting(settingsList.Value[0].Name).Value;

    std::cout << "Retrieved setting with name " << setting.Name << ", with value " << setting.Value
              << std::endl;

    Setting updatedSetting
        = settingsClient.UpdateSetting(settingsList.Value[0].Name, setting.Value).Value;

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

  return 0;
}
