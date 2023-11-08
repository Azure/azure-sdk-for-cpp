// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/storage/common/storage_credential.hpp"

#include <azure/core/http/policies/policy.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace _internal {
  namespace Test {
    class SharedKeyCredentialLiteTest_SharedKeyCredentialLite_Test;
    class SharedKeyCredentialLiteTest_SharedKeyCredentialLiteNoDate_Test;
    class SharedKeyCredentialLiteTest_SharedKeyCredentialLiteNoQuery_Test;
  } // namespace Test

  class SharedKeyPolicyLite final : public Core::Http::Policies::HttpPolicy {
    friend class Test::SharedKeyCredentialLiteTest_SharedKeyCredentialLite_Test;
    friend class Test::SharedKeyCredentialLiteTest_SharedKeyCredentialLiteNoDate_Test;
    friend class Test::SharedKeyCredentialLiteTest_SharedKeyCredentialLiteNoQuery_Test;

  public:
    explicit SharedKeyPolicyLite(std::shared_ptr<StorageSharedKeyCredential> credential)
        : m_credential(std::move(credential))
    {
    }

    ~SharedKeyPolicyLite() override {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<SharedKeyPolicyLite>(m_credential);
    }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Http::Request& request,
        Core::Http::Policies::NextHttpPolicy nextPolicy,
        Core::Context const& context) const override
    {
      request.SetHeader(
          "Authorization",
          "SharedKeyLite " + m_credential->AccountName + ":" + GetSignature(request));
      return nextPolicy.Send(request, context);
    }

  private:
    std::string GetSignature(const Core::Http::Request& request) const;
    std::shared_ptr<StorageSharedKeyCredential> m_credential;
  };

}}} // namespace Azure::Storage::_internal
