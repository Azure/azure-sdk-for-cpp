// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/http/policy.hpp>

#include "azure/storage/common/storage_credential.hpp"

namespace Azure { namespace Storage { namespace _detail {

  class SharedKeyPolicy : public Core::Http::Policies::HttpPolicy {
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
        Core::Http::Request& request,
        Core::Http::Policies::NextHttpPolicy nextHttpPolicy,
        Core::Context const& ctx) const override
    {
      request.SetHeader(
          "Authorization", "SharedKey " + m_credential->AccountName + ":" + GetSignature(request));
      return nextHttpPolicy.Send(request, ctx);
    }

  private:
    std::string GetSignature(const Core::Http::Request& request) const;

    std::shared_ptr<StorageSharedKeyCredential> m_credential;
  };

}}} // namespace Azure::Storage::_detail
