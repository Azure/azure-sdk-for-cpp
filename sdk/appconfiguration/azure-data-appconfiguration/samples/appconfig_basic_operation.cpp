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

// Retrieve a snapshot
static void RetrieveSnapshot(ConfigurationClient& configurationClient)
{
  // Current

  {
    Azure::Response<GetSnapshotResult> getSnapshotResult
        = configurationClient.GetSnapshot("snapshot-name", "accept");

    GetSnapshotResult result = getSnapshotResult.Value;

    std::cout << result.Name; // snapshot-name

    if (result.RetentionPeriod.HasValue())
    {
      std::cout << " : " << result.RetentionPeriod.Value();
    }

    if (result.Status.HasValue())
    {
      std::cout << " : " << result.Status.Value().ToString();
    }
    std::cout << std::endl;
  }

  // Expected

#if 0
  {
    Azure::Response<Snapshot> getSnapshotResult = configurationClient.GetSnapshot("snapshot-name");

    Snapshot snapshot = getSnapshotResult.Value;

    std::cout << snapshot.Name; // snapshot-name

    if (snapshot.RetentionPeriod.HasValue())
    {
      std::cout << " : " << snapshot.RetentionPeriod.Value();
    }

    if (snapshot.Status.HasValue())
    {
      std::cout << " : " << snapshot.Status.Value().ToString();
    }
    std::cout << std::endl;
  }
#endif
}

// Archive a snapshot
static void ArchiveSnapshot(ConfigurationClient& configurationClient)
{
  // Current

  {
    SnapshotUpdateParameters entity = {};
    entity.Status = SnapshotStatus::Archived;

    Azure::Response<UpdateSnapshotResult> updateSnapshotResult = configurationClient.UpdateSnapshot(
        UpdateSnapshotRequestContentType::ApplicationMergePatchJson,
        "snapshot-name",
        "accept",
        entity);

    UpdateSnapshotResult result = updateSnapshotResult.Value;

    std::cout << result.Name; // snapshot-name

    if (result.RetentionPeriod.HasValue())
    {
      std::cout << " : " << result.RetentionPeriod.Value();
    }

    if (result.Status.HasValue())
    {
      std::cout << " : " << result.Status.Value().ToString();
    }

    if (result.Expires.HasValue())
    {
      std::cout << " : " << result.Expires.Value();
    }

    std::cout << std::endl;
  }

  // Expected

#if 0
  {
    Azure::Response<Snapshot> archiveSnapshotResult
        = configurationClient.ArchiveSnapshot("snapshot-name");

    Snapshot snapshot = archiveSnapshotResult.Value;

    std::cout << snapshot.Name; // snapshot-name

    if (snapshot.RetentionPeriod.HasValue())
    {
      std::cout << " : " << snapshot.RetentionPeriod.Value();
    }

    if (snapshot.Status.HasValue())
    {
      std::cout << " : " << snapshot.Status.Value().ToString();
    }

    if (snapshot.Expires.HasValue())
    {
      std::cout << " : " << snapshot.Expires.Value();
    }

    std::cout << std::endl;
  }
#endif
}

// Recover a snapshot
static void RecoverSnapshot(ConfigurationClient& configurationClient)
{
  // Current

  {
    SnapshotUpdateParameters entity = {};
    entity.Status = SnapshotStatus::Ready;

    Azure::Response<UpdateSnapshotResult> updateSnapshotResult = configurationClient.UpdateSnapshot(
        UpdateSnapshotRequestContentType::ApplicationMergePatchJson,
        "snapshot-name",
        "accept",
        entity);

    UpdateSnapshotResult result = updateSnapshotResult.Value;

    std::cout << result.Name; // snapshot-name

    if (result.RetentionPeriod.HasValue())
    {
      std::cout << " : " << result.RetentionPeriod.Value();
    }

    if (result.Status.HasValue())
    {
      std::cout << " : " << result.Status.Value().ToString();
    }

    std::cout << " : Has expires value? " << result.Expires.HasValue() << std::endl;
  }

  // Expected

#if 0
  {
    Azure::Response<Snapshot> recoverSnapshotResult
        = configurationClient.RecoverSnapshot("snapshot-name");

    Snapshot snapshot = recoverSnapshotResult.Value;

    std::cout << snapshot.Name; // snapshot-name

    if (snapshot.RetentionPeriod.HasValue())
    {
      std::cout << " : " << snapshot.RetentionPeriod.Value();
    }

    if (snapshot.Status.HasValue())
    {
      std::cout << " : " << snapshot.Status.Value().ToString();
    }

    std::cout << " : Has expires value? " << snapshot.Expires.HasValue() << std::endl;
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

    // Retrieve a snapshot
    RetrieveSnapshot(configurationClient);

    // Archive a snapshot
    ArchiveSnapshot(configurationClient);

    // Recover a snapshot
    RecoverSnapshot(configurationClient);

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
