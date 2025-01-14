// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample provides the code implementation to use the App Configuration client SDK for
 * C++ to create, retrieve and delete a configuration setting.
 */

#include <azure/data/appconfiguration.hpp>
#include <azure/identity.hpp>

#include <iostream>

using namespace Azure::Data::AppConfiguration;
using namespace Azure::Identity;

// Retreive labels based on filters
static void RetrieveLabels(ConfigurationClient& configurationClient)
{
  // Current

  {
    GetLabelsOptions options;
    // To get all labels, don't set Name or use the any wildcard ("*").
    options.Name = "production*";
    options.AcceptDatetime = "Fri, 10 Jan 2025 00:00:00 GMT";

    for (GetLabelsPagedResponse labelsPage = configurationClient.GetLabels("accept", options);
         labelsPage.HasPage();
         labelsPage.MoveToNextPage())
    {
      if (labelsPage.Items.HasValue())
      {
        std::vector<Label> list = labelsPage.Items.Value();
        std::cout << "Label List Size: " << list.size() << std::endl;
        for (Label label : list)
        {
          if (label.Name.HasValue())
          {
            std::cout << label.Name.Value() << std::endl;
          }
        }
      }
    }
  }

  // Expected

#if 0
  {
    GetLabelsOptions options;
    // To get all labels, don't set Name or use the any wildcard ("*").
    options.Name = "production*";
    options.AcceptDatetime = Azure::DateTime(2025, 01, 10, 0, 0, 0);

    for (GetLabelsPagedResponse labelsPage = configurationClient.GetLabels(options);
         labelsPage.HasPage();
         labelsPage.MoveToNextPage())
    {
      if (labelsPage.Items.HasValue())
      {
        std::vector<Label> list = labelsPage.Items.Value();
        std::cout << "Label List Size: " << list.size() << std::endl;
        for (Label label : list)
        {
          if (label.Name.HasValue())
          {
            std::cout << label.Name.Value() << std::endl;
          }
        }
      }
    }
  }
#endif
}

// Retreive key values based on filters
static void RetrieveConfigurationSettings(ConfigurationClient& configurationClient)
{
  // Current

  {
    GetKeyValuesOptions options;
    options.Label = "production*";

    for (GetKeyValuesPagedResponse keyValuesPage
         = configurationClient.GetKeyValues("accept", options);
         keyValuesPage.HasPage();
         keyValuesPage.MoveToNextPage())
    {
      if (keyValuesPage.Items.HasValue())
      {
        std::vector<KeyValue> list = keyValuesPage.Items.Value();
        std::cout << "KeyValues List Size: " << list.size() << std::endl;
        for (KeyValue keyValue : list)
        {
          Azure::Nullable<std::string> valueOfKey = keyValue.Value;

          if (valueOfKey.HasValue())
          {
            std::cout << keyValue.Key << " : " << valueOfKey.Value() << std::endl;
          }
          else
          {
            std::cout << "Value for: '" << keyValue.Key << "' does not exist." << std::endl;
          }
        }
      }
    }
  }

  // Expected

#if 0
  {
    GetConfigurationSettingsOptions options;
    options.Label = "production*";

    for (GetConfigurationSettingsPagedResponse keyValuesPage
         = configurationClient.GetConfigurationSettings(options);
         keyValuesPage.HasPage();
         keyValuesPage.MoveToNextPage())
    {
      if (keyValuesPage.Items.HasValue())
      {
        std::vector<KeyValue> list = keyValuesPage.Items.Value();
        std::cout << "KeyValues List Size: " << list.size() << std::endl;
        for (KeyValue keyValue : list)
        {
          Azure::Nullable<std::string> valueOfKey = keyValue.Value;

          if (valueOfKey.HasValue())
          {
            std::cout << keyValue.Key << " : " << valueOfKey.Value() << std::endl;
          }
          else
          {
            std::cout << "Value for: '" << keyValue.Key << "' does not exist." << std::endl;
          }
        }
      }
    }
  }
#endif
}

// Retreive snapshots based on filters
static void RetrieveSnapshots(ConfigurationClient& configurationClient)
{
  // Current

  {
    GetSnapshotsOptions options;
    options.Name = "production*";
    options.Status = {SnapshotStatus::Ready, SnapshotStatus::Archived};

    for (GetSnapshotsPagedResponse snapshotsPage
         = configurationClient.GetSnapshots("accept", options);
         snapshotsPage.HasPage();
         snapshotsPage.MoveToNextPage())
    {
      if (snapshotsPage.Items.HasValue())
      {
        std::vector<Snapshot> list = snapshotsPage.Items.Value();
        std::cout << "Snapshot List Size: " << list.size() << std::endl;
        for (Snapshot snapshot : list)
        {
          std::cout << snapshot.Name;

          if (snapshot.RetentionPeriod.HasValue())
            std::cout << " : " << snapshot.RetentionPeriod.Value();

          if (snapshot.Status.HasValue())
            std::cout << " : " << snapshot.Status.Value().ToString();
          std::cout << std::endl;
        }
      }
    }
  }

  // Expected

#if 0
  {
    GetSnapshotsOptions options;
    options.Name = "production*";
    options.Status = {SnapshotStatus::Ready, SnapshotStatus::Archived};

    for (GetSnapshotsPagedResponse snapshotsPage = configurationClient.GetSnapshots(options);
         snapshotsPage.HasPage();
         snapshotsPage.MoveToNextPage())
    {
      if (snapshotsPage.Items.HasValue())
      {
        std::vector<Snapshot> list = snapshotsPage.Items.Value();
        std::cout << "Snapshot List Size: " << list.size() << std::endl;
        for (Snapshot snapshot : list)
        {
          std::cout << snapshot.Name;

          if (snapshot.RetentionPeriod.HasValue())
            std::cout << " : " << snapshot.RetentionPeriod.Value();

          if (snapshot.Status.HasValue())
            std::cout << " : " << snapshot.Status.Value().ToString();
          std::cout << std::endl;
        }
      }
    }
  }
#endif
}

// Retreive revisions based on filters
static void RetrieveRevisions(ConfigurationClient& configurationClient)
{
  // Current

  {
    GetRevisionsOptions options;
    options.Key = "some-key";

    for (GetRevisionsPagedResponse revisionsPage
         = configurationClient.GetRevisions("accept", options);
         revisionsPage.HasPage();
         revisionsPage.MoveToNextPage())
    {
      if (revisionsPage.Items.HasValue())
      {
        std::vector<KeyValue> list = revisionsPage.Items.Value();
        std::cout << "Revisions List Size: " << list.size() << std::endl;
        for (KeyValue keyValue : list)
        {
          Azure::Nullable<std::string> valueOfKey = keyValue.Value;
          if (valueOfKey.HasValue())
          {
            std::cout << keyValue.Key << " : " << valueOfKey.Value() << std::endl;
          }
          else
          {
            std::cout << "Value for: '" << keyValue.Key << "' does not exist." << std::endl;
          }
        }
      }
    }
  }

  // Expected

#if 0
  {
    GetRevisionsOptions options;
    options.Key = "some-key";

    for (GetRevisionsPagedResponse revisionsPage = configurationClient.GetRevisions(options);
         revisionsPage.HasPage();
         revisionsPage.MoveToNextPage())
    {
      if (revisionsPage.Items.HasValue())
      {
        std::vector<KeyValue> list = revisionsPage.Items.Value();
        std::cout << "Revisions List Size: " << list.size() << std::endl;
        for (KeyValue keyValue : list)
        {
          Azure::Nullable<std::string> valueOfKey = keyValue.Value;
          if (valueOfKey.HasValue())
          {
            std::cout << keyValue.Key << " : " << valueOfKey.Value() << std::endl;
          }
          else
          {
            std::cout << "Value for: '" << keyValue.Key << "' does not exist." << std::endl;
          }
        }
      }
    }
  }
#endif
}

int main()
{
  try
  {
    std::string url = "https://<your-appconfig-name>.azconfig.io";
    auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();

    // Create a ConfigurationClient
    ConfigurationClient configurationClient(url, credential);

    // Create a configuration setting

    // Current
    {
      KeyValue entity;
      entity.Value = "some-value";

      PutKeyValueOptions options;
      options.Label = "some-label";
      options.Entity = entity;

      Azure::Response<PutKeyValueResult> putKeyValueResult = configurationClient.PutKeyValue(
          PutKeyValueRequestContentType::ApplicationJson, "some-key", "accept", options);

      PutKeyValueResult result = putKeyValueResult.Value;
      Azure::Nullable<std::string> valueOfKey = result.Value;

      std::cout << result.Key << std::endl; // some-key

      if (valueOfKey.HasValue())
      {
        std::cout << valueOfKey.Value() << std::endl; // some-value
      }
    }

    // Expected

#if 0
    {
      ConfigurationSetting setting;
      setting.Key = "some-key";
      setting.Value = "some-value";

      SetSettingOptions options;
      options.Label = "some-label";

      Azure::Response<ConfigurationSetting> setResult
          = configurationClient.SetConfigurationSetting(setting, options);

      ConfigurationSetting result = setResult.Value;
      Azure::Nullable<std::string> valueOfKey = result.Value;

      std::cout << result.Key << std::endl; // some-key

      if (valueOfKey.HasValue())
      {
        std::cout << valueOfKey.Value() << std::endl; // some-value
      }
    }
#endif

    // Retrieve a configuration setting

    // Current
    {
      GetKeyValueOptions options;
      options.Label = "some-label";
      Azure::Response<GetKeyValueResult> getKeyValueResult
          = configurationClient.GetKeyValue("some-key", "accept", options);

      GetKeyValueResult result = getKeyValueResult.Value;
      Azure::Nullable<std::string> valueOfKey = result.Value;

      if (valueOfKey.HasValue())
      {
        std::cout << valueOfKey.Value() << std::endl; // some-value
      }
      else
      {
        std::cout << "Value for: '" << result.Key << "' does not exist." << std::endl;
      }
    }

    // Expected

#if 0
    {
      GetConfigurationSettingOptions options;
      options.Label = "some-label";
      Azure::Response<ConfigurationSetting> getResult
          = configurationClient.GetConfigurationSetting("some-key", options);

      ConfigurationSetting result = getResult.Value;
      Azure::Nullable<std::string> valueOfKey = result.Value;

      if (valueOfKey.HasValue())
      {
        std::cout << valueOfKey.Value() << std::endl; // some-value
      }
      else
      {
        std::cout << "Value for: '" << result.Key << "' does not exist." << std::endl;
      }
    }
#endif

    // Retreive labels based on filters
    RetrieveLabels(configurationClient);

    // Retreive configuration settings based on filters
    RetrieveConfigurationSettings(configurationClient);

    // Retreive snapshots based on filters
    RetrieveSnapshots(configurationClient);

    // Retreive revisions based on filters
    RetrieveRevisions(configurationClient);

    // Delete a configuration setting

    // Current
    {
      DeleteKeyValueOptions options;
      options.Label = "some-label";

      Azure::Response<DeleteKeyValueResult> deleteKeyValueResult
          = configurationClient.DeleteKeyValue("some-key", "accept", options);

      DeleteKeyValueResult result = deleteKeyValueResult.Value;
      Azure::Nullable<std::string> valueOfKey = result.Value;

      if (valueOfKey.HasValue())
      {
        std::cout << valueOfKey.Value() << std::endl; // some-value
      }
      else
      {
        std::cout << "Value for: '" << result.Key << "' does not exist." << std::endl;
      }
    }

    // Expected

#if 0
    {
      DeleteKeyValueOptions options;
      options.Label = "some-label";

      Azure::Response<ConfigurationSetting> deleteKeyValueResult
          = configurationClient.DeleteConfigurationSetting("some-key", options);

      ConfigurationSetting result = deleteKeyValueResult.Value;
      Azure::Nullable<std::string> valueOfKey = result.Value;

      if (valueOfKey.HasValue())
      {
        std::cout << valueOfKey.Value() << std::endl; // some-value
      }
      else
      {
        std::cout << "Value for: '" << result.Key << "' does not exist." << std::endl;
      }
    }
#endif
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication error:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Client request failed error:" << std::endl << e.what() << std::endl;
    return 1;
  }

  return 0;
}
