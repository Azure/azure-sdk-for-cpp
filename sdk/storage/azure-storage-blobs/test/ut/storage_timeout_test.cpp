// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <chrono>
#include <memory>
#include <vector>

#include <azure/storage/blobs.hpp>

#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class PeekHttpRequestPolicy final : public Core::Http::Policies::HttpPolicy {
  public:
    PeekHttpRequestPolicy(std::function<void(const Core::Http::Request&)> callback)
        : m_callback(std::move(callback))
    {
    }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Http::Request& request,
        Core::Http::Policies::NextHttpPolicy nextPolicy,
        Core::Context const& context) const override
    {
      m_callback(request);
      return nextPolicy.Send(request, context);
    }

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<PeekHttpRequestPolicy>(*this);
    }

  private:
    std::function<void(const Core::Http::Request&)> m_callback;
  };

  TEST_F(StorageTest, StoragetimeoutTestBasic)
  {
    Azure::Nullable<int64_t> timeout;
    auto callback = [&timeout](const Core::Http::Request& request) {
      auto queryParameteres = request.GetUrl().GetQueryParameters();
      auto ite = queryParameteres.find("timeout");
      if (ite == queryParameteres.end())
      {
        timeout.Reset();
      }
      else
      {
        timeout = std::stoll(ite->second);
      }
    };

    auto peekPolicyPtr = std::make_unique<PeekHttpRequestPolicy>(callback);
    Blobs::BlobClientOptions clientOptions = InitStorageClientOptions<Blobs::BlobClientOptions>();
    clientOptions.PerRetryPolicies.emplace_back(std::move(peekPolicyPtr));
    auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(), LowercaseRandomString(), clientOptions);
    containerClient.DeleteIfExists();
    EXPECT_FALSE(timeout.HasValue());

    Azure::Core::Context context;
    context = context.WithDeadline(std::chrono::system_clock::now() + std::chrono::seconds(300));
    containerClient.DeleteIfExists(Storage::Blobs::DeleteBlobContainerOptions(), context);

    ASSERT_TRUE(timeout.HasValue());
    EXPECT_GE(timeout.Value(), 299);
    EXPECT_LE(timeout.Value(), 301);
  }

  TEST_F(StorageTest, StoragetimeoutTest_Cancelled)
  {
    Blobs::BlobClientOptions clientOptions;
    auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(), LowercaseRandomString(), clientOptions);

    Azure::Core::Context context;
    context.Cancel();
    // Should not throw an error on time point casting.
    EXPECT_THROW(
        containerClient.DeleteIfExists(Storage::Blobs::DeleteBlobContainerOptions(), context),
        Core::OperationCancelledException);
  }

}}} // namespace Azure::Storage::Test
