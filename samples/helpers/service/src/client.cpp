// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/service/client.hpp"

#include <azure/core/internal/environment.hpp>
#include <azure/core/internal/strings.hpp>

void Azure::Service::Client::DoSomething(const Azure::Core::Context& context) const
{
  static_cast<void>(context); // to suppress the "unused variable" warning.

  if (!Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
          Core::_internal::Environment::GetVariable("AZURE_SDK_IDENTITY_SAMPLE_SERVICE_GETTOKEN"),
          "disable"))
  {
    // An oversimplified logic of what a typical Azure SDK client does is below:
    // Every client has its own scope. We use management.azure.com here as an example.
    Core::Credentials::TokenRequestContext azureServiceClientContext;
    azureServiceClientContext.Scopes = {"https://management.azure.com/.default"};

    auto authenticationToken = m_credential->GetToken(azureServiceClientContext, context);

    // Now that it has a token, Client can authorize and DoSomething().
    // ...
    // ...

    static_cast<void>(authenticationToken); // to suppress the "unused variable" warning.
  }
}
