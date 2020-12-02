// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/http/policy.hpp"
#include "azure/storage/common/storage_credential.hpp"

namespace Azure { namespace Storage { namespace Details {

  class SharedKeyPolicy : public Core::Http::HttpPolicy {
  public:
    explicit SharedKeyPolicy(std::shared_ptr<StorageSharedKeyCredential> credential)
        : m_credential(std::move(credential))
    {
    }

    ~SharedKeyPolicy() override {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<SharedKeyPolicy>(m_credential);
    }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Context const& ctx,
        Core::Http::Request& request,
        Core::Http::NextHttpPolicy nextHttpPolicy) const override
    {
      request.AddHeader(
          "Authorization", "SharedKey " + m_credential->AccountName + ":" + GetSignature(request));
      return nextHttpPolicy.Send(ctx, request);
    }

  private:
    std::string GetSignature(const Core::Http::Request& request) const;

    std::shared_ptr<StorageSharedKeyCredential> m_credential;
  };

}}} // namespace Azure::Storage::Details
