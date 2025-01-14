// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample provides the code implementation to use the App Configuration client SDK for
 * C++ to create, retrieve and delete a configuration setting.
 */

#include <azure/data/appconfiguration.hpp>
#include <azure/identity.hpp>

#include <iostream>
#include <thread>

using namespace Azure::Data::AppConfiguration;
using namespace Azure::Identity;

// Retreive labels based on filters
static void RetreiveLabels(ConfigurationClient& configurationClient)
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

// Create a snapshot
static void CreateSnapshot(ConfigurationClient& configurationClient)
{
  // Current

  {
    KeyValueFilter filter;
    filter.Key = "*";

    Snapshot entity;
    entity.Filters = {filter};
    entity.RetentionPeriod = 3600; // 1 hour, minimum allowed value

    CreateSnapshotOptions options;

    Azure::Response<CreateSnapshotResult> createSnapshotResult = configurationClient.CreateSnapshot(
        CreateSnapshotRequestContentType ::ApplicationJson,
        "snapshot-name",
        "accept",
        entity,
        options);

    CreateSnapshotResult result = createSnapshotResult.Value;

    if (result.Status.HasValue())
    {
      std::cout << " : " << result.Status.Value().ToString() << std::endl; // Provisioning
    }

    // Manually poll for up to a maximum of 30 seconds.
    GetOperationDetailsOptions getDetailsOptions;
    for (int i = 0; i < 30; i++)
    {
      Azure::Response<OperationDetails> details
          = configurationClient.GetOperationDetails("snapshot-name", getDetailsOptions);

      std::cout << details.Value.Status.ToString() << std::endl;
      if (details.Value.Status == OperationState::Succeeded)
      {
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::cout << result.Name; // snapshot-name

    if (result.RetentionPeriod.HasValue())
    {
      std::cout << " : " << result.RetentionPeriod.Value();
    }

    if (result.Status.HasValue())
    {
      // This should be Ready, but there's no way to update the LRO result with the current
      // approach. So, it incorrectly states Provisioning.
      std::cout << " : " << result.Status.Value().ToString();
    }
    std::cout << std::endl;
  }

  // Expected

#if 0
  {
    KeyValueFilter filter;
    filter.Key = "*";

    Snapshot entity;
    entity.Filters = {filter};
    entity.RetentionPeriod = 3600; // 1 hour, minimum allowed value

    StartCreateSnapshotOptions options;

    StartCreateSnapshotOperation createSnapshotOperation
        = configurationClient.StartCreateSnapshot("snapshot-name", entity, options);

    // Waits for the operation to finish, checking for status every 1 second.
    Azure::Response<Snapshot> snapshotResult
        = createSnapshotOperation.PollUntilDone(std::chrono::milliseconds(1000));
    Snapshot snapshot = snapshotResult.Value;

    std::cout << snapshot.Name; // snapshot-name

    if (snapshot.RetentionPeriod.HasValue())
    {
      std::cout << " : " << snapshot.RetentionPeriod.Value();
    }

    if (snapshot.Status.HasValue())
    {
      std::cout << " : " << snapshot.Status.Value().ToString(); // Ready
    }
    std::cout << std::endl;
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
    RetreiveLabels(configurationClient);

    // Create a snapshot
    CreateSnapshot(configurationClient);

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
