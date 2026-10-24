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
    Queues::ListQueuesOptions list;
    EXPECT_FALSE(list.Prefix.HasValue());
    EXPECT_FALSE(list.ContinuationToken.HasValue());
    EXPECT_FALSE(list.PageSizeHint.HasValue());
    EXPECT_EQ(list.Include, Models::ListQueuesIncludeFlags::None);
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
