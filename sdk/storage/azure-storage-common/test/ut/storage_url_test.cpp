// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test_base.hpp"

#include <azure/storage/common/internal/storage_url.hpp>

namespace Azure { namespace Storage { namespace Test {

  TEST(StorageUrlTest, General)
  {
    auto res = _internal::ParseStorageUrl(Azure::Core::Url("https://example.com"));
    EXPECT_FALSE(res.HasValue());

    res = _internal::ParseStorageUrl(
        Azure::Core::Url("https://account.blob.core.windows.net/container"));
    EXPECT_TRUE(res.HasValue());
    EXPECT_EQ(res.Value().AccountName, "account");
    EXPECT_EQ(res.Value().Service, "blob");
    EXPECT_TRUE(res.Value().ContainerName.HasValue());
    EXPECT_EQ(res.Value().ContainerName.Value(), "container");
    EXPECT_EQ(res.Value().ContainerUrl.Value(), "https://account.blob.core.windows.net/container");

    res = _internal::ParseStorageUrl(
        Azure::Core::Url("http://account.dfs.preprod.core.windows.net/container/foo/bar/blob"));
    EXPECT_TRUE(res.HasValue());
    EXPECT_EQ(res.Value().AccountName, "account");
    EXPECT_EQ(res.Value().Service, "dfs");
    EXPECT_TRUE(res.Value().ContainerName.HasValue());
    EXPECT_EQ(res.Value().ContainerName.Value(), "container");
    EXPECT_EQ(res.Value().ContainerUrl.Value(), "http://account.dfs.preprod.core.windows.net/container");

    res = _internal::ParseStorageUrl(
        Azure::Core::Url("https://account.file.core.windows.net/container/"));
    EXPECT_TRUE(res.HasValue());
    EXPECT_EQ(res.Value().AccountName, "account");
    EXPECT_EQ(res.Value().Service, "file");
    EXPECT_TRUE(res.Value().ContainerName.HasValue());
    EXPECT_EQ(res.Value().ContainerName.Value(), "container");
    EXPECT_EQ(res.Value().ContainerUrl.Value(), "https://account.file.core.windows.net/container");

    res = _internal::ParseStorageUrl(Azure::Core::Url("http://account.queue.core.windows.net/"));
    EXPECT_TRUE(res.HasValue());
    EXPECT_EQ(res.Value().AccountName, "account");
    EXPECT_EQ(res.Value().Service, "queue");
    EXPECT_FALSE(res.Value().ContainerName.HasValue());
    EXPECT_FALSE(res.Value().ContainerUrl.HasValue());

    res = _internal::ParseStorageUrl(Azure::Core::Url("https://account.table.core.windows.net"));
    EXPECT_TRUE(res.HasValue());
    EXPECT_EQ(res.Value().AccountName, "account");
    EXPECT_EQ(res.Value().Service, "table");
    EXPECT_FALSE(res.Value().ContainerName.HasValue());
    EXPECT_FALSE(res.Value().ContainerUrl.HasValue());
  }

}}} // namespace Azure::Storage::Test
