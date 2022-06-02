// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/credentials/credentials.hpp>

#include <memory>
#include <utility>

namespace Azure { namespace Service {

  // This class is an oversimplified placeholder that is intended to represent an Azure SDK service
  // client that uses Azure::Core::Credentials::TokenCredential to authenticate: Key Vault client,
  // Storage Blobs client, etc.
  class Client final {
  private:
    std::shared_ptr<Core::Credentials::TokenCredential> m_credential;

  public:
    explicit Client(
        const std::string& serviceUrl,
        std::shared_ptr<Core::Credentials::TokenCredential> credential)
        : m_credential(std::move(credential))
    {
      static_cast<void>(serviceUrl); // to suppress the "unused variable" warning.
    }

    void DoSomething(const Core::Context& context) const
    {
      static_cast<void>(context); // to suppress the "unused variable" warning.

      // This method does nothing, because the purpose of this class is to demonstrate
      // how Azure::Identity classes can be used with a generic Azure SDK service client.
      // If we have code here that gets the token, it would be up to the user to set it up to be
      // valid enough to get a token, which is not critical for the intended demonstration purposes.
      // And if user runs this, and authentication is unsuccessful, it may draw an unnecessary
      // attention to an irrelevant (to the demo) point.

      // But an oversimplified logic of what a typical Azure SDK client does is below:
#if (0)
      // Every client has its own scope. We use management.azure.com here as an example.
      Core::Credentials::TokenRequestContext azureServiceClientContext;
      azureServiceClientContext.Scopes = {"https://management.azure.com/"};

      auto authenticationToken = m_credential->GetToken(azureServiceClientContext, context);

      // Now that it has a token, Client can authorize and DoSomething().
      // ...
      // ...

      static_cast<void>(authenticationToken); // to suppress the "unused variable" warning.
#endif
    }
  };

}} // namespace Azure::Service
