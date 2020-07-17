// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_credential.hpp"
#include "http/policy.hpp"

namespace Azure { namespace Storage {

  class SharedKeyPolicy : public Core::Http::HttpPolicy {
  public:
    explicit SharedKeyPolicy(std::shared_ptr<SharedKeyCredential> credential)
        : m_credential(std::move(credential))
    {
    }

    ~SharedKeyPolicy() override {}

    HttpPolicy* Clone() const override { return new SharedKeyPolicy(m_credential); }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Context& ctx,
        Core::Http::Request& request,
        Core::Http::NextHttpPolicy nextHttpPolicy) const override
    {
      request.AddHeader(
          "Authorization", "SharedKey " + m_credential->AccountName + ":" + GetSignature(request));
      return nextHttpPolicy.Send(ctx, request);
    }

  private:
    std::string GetSignature(const Core::Http::Request& request) const;

    std::shared_ptr<SharedKeyCredential> m_credential;
  };

}} // namespace Azure::Storage
