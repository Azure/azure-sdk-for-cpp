# _Getting, updating, settings

This sample demonstrates how to get one or more settings and update a setting in Azure Key Vault.
To get started, you'll need a URI to an Azure Key Vault HSM.

## _Creating a SettingsClient

To create a new `SettingsClient` to access settings, you need the endpoint to an Azure Key Vault HSM and credentials.

Key Vault Settings client for C++ currently supports the `ClientSecretCredential` for authenticating.

In the sample below, you can create a credential by setting the Tenant ID, Client ID and Client Secret as environment variables.

```cpp Snippet:SampleAdministration1CreateCredential
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:SampleAdministration2SettingsClient
  // create client
  SettingsClient settingsClient(std::getenv("AZURE_KEYVAULT_HSM_URL"), credential);
```

## _Get Settings

Call GetSettings get a list of all keyvault settings stored in the HSM.

```cpp Snippet:SampleAdministration3GetSettings
  // Get all settings 
  SettingsListResult settingsList = settingsClient.GetSettings().Value;
  std::cout << "Number of settings found : " << settingsList.Value.size();
```

## _Getting a setting by name

Call GetSetting to retrieve a setting from HSM by passing the setting name as parameter. In this example we use a name from the list obtained previously.

```cpp Snippet:SampleAdministration4GetSetting
  Setting setting = settingsClient.GetSetting(settingsList.Value[0].Name).Value;
  std::cout << "Retrieved setting with name " << setting.Name << ", with value " << setting.Value;
```

## _Updating a setting

Call UpdateSetting to modify an existing setting. Create an options object and initialize its Value field with the new value. In this example we do not change the actual value in order to not affect the keyvault long term.


```cpp Snippet:SampleAdministration5UpdateSetting
  UpdateSettingOptions options; 
  options.Value = setting.Value;
  Setting updatedSetting = settingsClient.UpdateSetting(settingsList.Value[0].Name,options).Value;
  std::cout << "Retrieved updated setting with name " << updatedSetting.Name << ", with value " << updatedSetting.Value;
```

## _Source

To see the full example source, see:
[Source Code](https://github.com/gearama/azure-sdk-for-cpp/tree/settings/sdk/keyvault/azure-security-keyvault-administration/test/samples/sample1-basic-operations)
