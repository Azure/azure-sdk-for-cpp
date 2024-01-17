# Getting, updating, settings

This sample demonstrates how to get one or more settings and update a setting in Azure Key Vault.
To get started, you'll need a URI to an Azure Key Vault HSM.

## Creating a SettingsClient

To create a new `SettingsClient` to access settings, you need the endpoint to an Azure Key Vault HSM and credentials.

Key Vault Settings client for C++ currently supports any `TokenCredential` for authenticating.

```cpp Snippet:SampleAdministration1CreateCredential
  auto credential
      = std::make_shared<Azure::Identity::DefaultAzureCredential>();
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:SampleAdministration2SettingsClient
  // create client
  SettingsClient settingsClient(std::getenv("AZURE_KEYVAULT_HSM_URL"), credential);
```

## Get Settings

Call GetSettings get a list of all keyvault settings stored in the HSM.

```cpp Snippet:SampleAdministration3GetSettings
  // Get all settings 
  SettingsListResult settingsList = settingsClient.GetSettings().Value;
  std::cout << "Number of settings found : " << settingsList.Value.size();
```

## Getting a setting by name

Call GetSetting to retrieve a setting from HSM by passing the setting name as parameter. In this example we use a name from the list obtained previously.

```cpp Snippet:SampleAdministration4GetSetting
  Setting setting = settingsClient.GetSetting(settingsList.Value[0].Name).Value;
  std::cout << "Retrieved setting with name " << setting.Name << ", with value " << setting.Value;
```

## Updating a setting

Call UpdateSetting to modify an existing setting. Create an options object and initialize its Value field with the new value. In this example we do not change the actual value in order to not affect the keyvault long term.


```cpp Snippet:SampleAdministration5UpdateSetting
  UpdateSettingOptions options; 
  options.Value = setting.Value;
  Setting updatedSetting = settingsClient.UpdateSetting(settingsList.Value[0].Name,options).Value;
  std::cout << "Retrieved updated setting with name " << updatedSetting.Name << ", with value " << updatedSetting.Value;
```

## Source

To see the full example source, see:
[Source Code](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-administration/test/samples/sample1-basic-operations)
