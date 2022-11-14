//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/service/client.hpp"

void Azure::Service::Client::DoSomething(const Azure::Core::Context& context) const
{
  static_cast<void>(context); // to suppress the "unused variable" warning.

  // An oversimplified logic of what a typical Azure SDK client does is below:
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
