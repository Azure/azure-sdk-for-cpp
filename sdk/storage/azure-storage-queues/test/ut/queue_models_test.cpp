// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/storage/queues/queue_responses.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test {

  TEST(QueueModels, Defaults)
  {
    namespace Models = Queues::Models;
    Models::RetentionPolicy retention;
    EXPECT_FALSE(retention.IsEnabled);
    EXPECT_FALSE(retention.Days.HasValue());
    Models::AnalyticsLogging logging;
    EXPECT_TRUE(logging.Version.empty());
    EXPECT_FALSE(logging.Delete);
    EXPECT_FALSE(logging.Read);
    EXPECT_FALSE(logging.Write);
    EXPECT_FALSE(logging.RetentionPolicy.IsEnabled);
    Models::Metrics metrics;
    EXPECT_TRUE(metrics.Version.empty());
    EXPECT_FALSE(metrics.IsEnabled);
    EXPECT_FALSE(metrics.IncludeApis.HasValue());
    EXPECT_FALSE(metrics.RetentionPolicy.IsEnabled);
    EXPECT_EQ(Models::CorsRule().MaxAgeInSeconds, 0);
    EXPECT_TRUE(Models::QueueServiceProperties().Cors.empty());
    EXPECT_TRUE(Models::GeoReplicationStatus().ToString().empty());
    EXPECT_FALSE(Models::GeoReplication().LastSyncedOn.HasValue());
    EXPECT_FALSE(Models::UserDelegationKey().SignedDelegatedUserTid.HasValue());
    EXPECT_TRUE(Models::QueueItem().Metadata.empty());
    EXPECT_FALSE(Models::CreateQueueResult().Created);
    EXPECT_TRUE(Models::DeleteQueueResult().Deleted);
    Models::SignedIdentifier identifier;
    EXPECT_FALSE(identifier.StartsOn.HasValue());
    EXPECT_FALSE(identifier.ExpiresOn.HasValue());
    EXPECT_TRUE(identifier.Permissions.empty());
    EXPECT_TRUE(Models::QueueAccessPolicy().SignedIdentifiers.empty());
    EXPECT_EQ(Models::QueueMessage().DequeueCount, 0);
    EXPECT_TRUE(Models::ReceivedMessages().Messages.empty());
    EXPECT_EQ(Models::PeekedQueueMessage().DequeueCount, 0);
    EXPECT_TRUE(Models::PeekedMessages().Messages.empty());
    Models::QueueProperties properties;
    EXPECT_TRUE(properties.Metadata.empty());
    EXPECT_EQ(properties.ApproximateMessageCount, 0);
    EXPECT_EQ(properties.ApproximateMessageCountLong, 0);
    Models::ServiceStatistics statistics;
    EXPECT_TRUE(statistics.GeoReplication.Status.ToString().empty());
    EXPECT_FALSE(statistics.GeoReplication.LastSyncedOn.HasValue());
    Queues::ListQueuesOptions list;
    EXPECT_FALSE(list.Prefix.HasValue());
    EXPECT_FALSE(list.ContinuationToken.HasValue());
    EXPECT_FALSE(list.PageSizeHint.HasValue());
    EXPECT_EQ(list.Include, Models::ListQueuesIncludeFlags::None);
  }

  TEST(QueueModels, ServicePropertiesDefaults)
  {
    Queues::Models::QueueServiceProperties properties;
    EXPECT_TRUE(properties.Cors.empty());
    EXPECT_TRUE(properties.Logging.Version.empty());
    EXPECT_FALSE(properties.Logging.Delete);
    EXPECT_FALSE(properties.Logging.Read);
    EXPECT_FALSE(properties.Logging.Write);
    EXPECT_FALSE(properties.Logging.RetentionPolicy.IsEnabled);
    EXPECT_FALSE(properties.Logging.RetentionPolicy.Days.HasValue());
    for (auto const* metrics : {&properties.HourMetrics, &properties.MinuteMetrics})
    {
      EXPECT_TRUE(metrics->Version.empty());
      EXPECT_FALSE(metrics->IsEnabled);
      EXPECT_FALSE(metrics->IncludeApis.HasValue());
      EXPECT_FALSE(metrics->RetentionPolicy.IsEnabled);
      EXPECT_FALSE(metrics->RetentionPolicy.Days.HasValue());
    }
  }

  TEST(QueueModels, MetadataIsCaseInsensitive)
  {
    Queues::Models::QueueItem item;
    Queues::Models::QueueProperties properties;
    for (auto* metadata : {&item.Metadata, &properties.Metadata})
    {
      EXPECT_TRUE(metadata->emplace("MixedCase", "original").second);
      auto const found = metadata->find("mixedcase");
      ASSERT_NE(found, metadata->end());
      EXPECT_EQ(found->second, "original");
      EXPECT_FALSE(metadata->emplace("MIXEDCASE", "duplicate").second);
      EXPECT_EQ(metadata->size(), 1U);
      (*metadata)["mIxEdCaSe"] = "updated";
      EXPECT_EQ(metadata->size(), 1U);
      EXPECT_EQ(metadata->at("MIXEDCASE"), "updated");
      EXPECT_EQ(metadata->at("mixedcase"), "updated");
    }
  }

  TEST(QueueModels, MessageResults)
  {
    auto const inserted = DateTime::Parse("2026-10-24T01:02:03Z", DateTime::DateFormat::Rfc3339);
    auto const expires = DateTime::Parse("2026-10-25T01:02:03Z", DateTime::DateFormat::Rfc3339);
    auto const visible = DateTime::Parse("2026-10-24T01:03:03Z", DateTime::DateFormat::Rfc3339);
    Queues::Models::EnqueueMessageResult enqueued{
        "message-id", inserted, expires, "receipt", visible};
    EXPECT_EQ(enqueued.MessageId, "message-id");
    EXPECT_EQ(enqueued.InsertedOn, inserted);
    EXPECT_EQ(enqueued.ExpiresOn, expires);
    EXPECT_EQ(enqueued.PopReceipt, "receipt");
    EXPECT_EQ(enqueued.NextVisibleOn, visible);
    Queues::Models::UpdateMessageResult updated{"updated-receipt", expires};
    EXPECT_EQ(updated.PopReceipt, "updated-receipt");
    EXPECT_EQ(updated.NextVisibleOn, expires);
    EXPECT_EQ(enqueued.PopReceipt, "receipt");
    EXPECT_EQ(enqueued.NextVisibleOn, visible);
  }

  TEST(QueueModels, Enumerations)
  {
    using Queues::Models::GeoReplicationStatus;
    using Queues::Models::ListQueuesIncludeFlags;
    EXPECT_EQ(GeoReplicationStatus::Live, GeoReplicationStatus("live"));
    EXPECT_EQ(GeoReplicationStatus::Bootstrap, GeoReplicationStatus("bootstrap"));
    EXPECT_EQ(GeoReplicationStatus::Unavailable, GeoReplicationStatus("unavailable"));
    EXPECT_EQ(GeoReplicationStatus("custom").ToString(), "custom");
    auto flags = ListQueuesIncludeFlags::None;
    EXPECT_EQ(static_cast<int>(flags), 0);
    EXPECT_EQ(static_cast<int>(ListQueuesIncludeFlags::Metadata), 1);
    flags |= ListQueuesIncludeFlags::Metadata;
    EXPECT_EQ(flags, ListQueuesIncludeFlags::Metadata);
    EXPECT_EQ(flags & ListQueuesIncludeFlags::Metadata, ListQueuesIncludeFlags::Metadata);
    EXPECT_EQ(flags | ListQueuesIncludeFlags::None, ListQueuesIncludeFlags::Metadata);
    flags &= ListQueuesIncludeFlags::None;
    EXPECT_EQ(flags, ListQueuesIncludeFlags::None);
  }

}}} // namespace Azure::Storage::Test
