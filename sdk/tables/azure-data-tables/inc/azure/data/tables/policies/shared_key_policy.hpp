// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/data/tables/credentials/shared_key_credential.hpp"

#include <azure/core/http/policies/policy.hpp>

#include <memory>
#include <string>

using namespace Azure::Data::Tables::Credentials;

namespace Azure { namespace Data { namespace Tables { namespace _internal { namespace Policies {

  class SharedKeyPolicy final : public Core::Http::Policies::HttpPolicy {
  public:
    explicit SharedKeyPolicy(std::shared_ptr<SharedKeyCredential> credential)
        : m_credential{std::move(credential)}
    {
    }

    ~SharedKeyPolicy() override {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<SharedKeyPolicy>(m_credential);
    }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Http::Request& request,
        Core::Http::Policies::NextHttpPolicy nextPolicy,
        Core::Context const& context) const override
    {
      request.SetHeader(
          "Authorization", "SharedKey " + m_credential->AccountName + ":" + GetSignature(request));
      return nextPolicy.Send(request, context);
    }

  private:
    std::string GetSignature(const Core::Http::Request& request) const;

    std::shared_ptr<SharedKeyCredential> m_credential;
  };

}}}}} // namespace Azure::Core::Http::Policies::_internal
