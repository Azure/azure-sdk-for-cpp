// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_credential.hpp"
#include "http/policy.hpp"

namespace Azure { namespace Storage {

  class TokenCredentialPolicy : public Core::Http::HttpPolicy {
  public:
    explicit TokenCredentialPolicy(std::shared_ptr<TokenCredential> credential)
        : m_credential(std::move(credential))
    {
    }

    ~TokenCredentialPolicy() override {}

    HttpPolicy* Clone() const override { return new TokenCredentialPolicy(m_credential); }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Context& ctx,
        Core::Http::Request& request,
        Core::Http::NextHttpPolicy nextHttpPolicy) const override
    {
      request.AddHeader("Authorization", "Bearer " + m_credential->GetToken());
      return nextHttpPolicy.Send(ctx, request);
    }

  private:
    std::shared_ptr<TokenCredential> m_credential;
  };

}} // namespace Azure::Storage
