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

// Make the setting read-only
static void SetReadOnly(ConfigurationClient& configurationClient)
{
  // Current

  {
    PutLockOptions options;
    options.Label = "some-label";

    Azure::Response<PutLockResult> putLockResult
        = configurationClient.PutLock("some-key", "accept", options);

    PutLockResult result = putLockResult.Value;
    Azure::Nullable<bool> isLocked = result.Locked;

    std::cout << result.Key << std::endl; // some-key

    if (isLocked.HasValue())
    {
      std::cout << "isLocked: " << isLocked.Value() << std::endl; // true
    }
  }

  // Expected

#if 0
  {
    SetReadOnlyOptions options;
    options.Label = "some-label";

    Azure::Response<ConfigurationSetting> setReadOnlyResult
        = configurationClient.SetReadOnly("some-key", true, options);

    ConfigurationSetting result = setReadOnlyResult.Value;
    Azure::Nullable<bool> isReadOnly = result.IsReadOnly;

    std::cout << result.Key << std::endl; // some-key

    if (isReadOnly.HasValue())
    {
      std::cout << "isReadOnly: " << isReadOnly.Value() << std::endl; // true
    }
  }
#endif
}

// Try modifying a read-only setting and then modify a read-write setting
static void SetConfigurationSetting(ConfigurationClient& configurationClient)
{
  // Current
  {
    KeyValue entity;
    entity.Value = "another-value";

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
      std::cout << valueOfKey.Value() << std::endl; // another-value
    }
  }

  // Expected

#if 0
  {
    ConfigurationSetting setting;
    setting.Key = "some-key";
    setting.Value = "another-value";

    SetSettingOptions options;
    options.Label = "some-label";

    Azure::Response<ConfigurationSetting> setResult
        = configurationClient.SetConfigurationSetting(setting, options);

    ConfigurationSetting result = setResult.Value;
    Azure::Nullable<std::string> valueOfKey = result.Value;

    std::cout << result.Key << std::endl; // some-key

    if (valueOfKey.HasValue())
    {
      std::cout << valueOfKey.Value() << std::endl; // another-value
    }
  }
#endif
}

// Make the setting read-write
static void SetReadWrite(ConfigurationClient& configurationClient)
{
  // Current

  {
    DeleteLockOptions options;
    options.Label = "some-label";

    Azure::Response<DeleteLockResult> deleteLockResult
        = configurationClient.DeleteLock("some-key", "accept", options);

    DeleteLockResult result = deleteLockResult.Value;
    Azure::Nullable<bool> isLocked = result.Locked;

    std::cout << result.Key << std::endl; // some-key

    if (isLocked.HasValue())
    {
      std::cout << "isLocked: " << isLocked.Value() << std::endl; // false
    }
  }

  // Expected

#if 0
  {
    SetReadOnlyOptions options;
    options.Label = "some-label";

    Azure::Response<ConfigurationSetting> setReadOnlyResult
        = configurationClient.SetReadOnly("some-key", false, options);

    ConfigurationSetting result = setReadOnlyResult.Value;
    Azure::Nullable<bool> isReadOnly = result.IsReadOnly;

    std::cout << result.Key << std::endl; // some-key

    if (isReadOnly.HasValue())
    {
      std::cout << "isReadOnly: " << isReadOnly.Value() << std::endl; // false
    }
  }
#endif
}

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

// Retreive configuration settings for a snapshot
static void RetrieveConfigurationSettingsForSnapshot(ConfigurationClient& configurationClient)
{
  // Current

  {
    GetKeyValuesOptions options;
    options.Snapshot = "snapshot-name";

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
    options.Snapshot = "snapshot-name";

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
      std::cout << result.Status.Value().ToString() << std::endl; // Provisioning
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

    Azure::Response<GetSnapshotResult> getSnapshotResult
        = configurationClient.GetSnapshot("snapshot-name", "accept");

    GetSnapshotResult snapshot = getSnapshotResult.Value;

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

    Snapshot result = createSnapshotOperation.GetInitialResult();

    if (result.Status.HasValue())
    {
      std::cout << result.Status.Value().ToString() << std::endl; // Provisioning
    }

    // Waits for the operation to finish, checking for status every 1 second.
    Azure::Response<OperationDetails> operationResult
        = createSnapshotOperation.PollUntilDone(std::chrono::milliseconds(1000));

    std::cout << operationResult.Value.Status.ToString() << std::endl; // Succeeded

    Azure::Response<Snapshot> getSnapshotResult = configurationClient.GetSnapshot("snapshot-name");

    Snapshot snapshot = getSnapshotResult.Value;

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

    // Make the setting read-only
    SetReadOnly(configurationClient);

    try
    {
      // Trying to modify a read-only setting is expected to throw an exception because it cannot be
      // updated.
      SetConfigurationSetting(configurationClient);
    }
    catch (Azure::Core::RequestFailedException const& e)
    {
      std::cout << "Client request failed error when trying to modify a read-only setting:"
                << std::endl
                << e.what() << std::endl;
    }

    // Make the setting read-write
    SetReadWrite(configurationClient);

    // Trying to modify a read-write setting should succeed
    SetConfigurationSetting(configurationClient);

    // Retreive labels based on filters
    RetrieveLabels(configurationClient);

    // Retreive configuration settings based on filters
    RetrieveConfigurationSettings(configurationClient);

    // Retreive configuration settings for a snapshot
    RetrieveConfigurationSettingsForSnapshot(configurationClient);

    // Retreive snapshots based on filters
    RetrieveSnapshots(configurationClient);

    // Retreive revisions based on filters
    RetrieveRevisions(configurationClient);

    // Retrieve a snapshot
    RetrieveSnapshot(configurationClient);

    // Archive a snapshot
    ArchiveSnapshot(configurationClient);

    // Recover a snapshot
    RecoverSnapshot(configurationClient);

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
        std::cout << valueOfKey.Value() << std::endl; // another-value
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
        std::cout << valueOfKey.Value() << std::endl; // another-value
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
