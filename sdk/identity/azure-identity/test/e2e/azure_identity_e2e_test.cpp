// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/internal/environment.hpp>
#include <azure/core/diagnostics/logger.hpp>
#include <azure/identity/managed_identity_credential.hpp>

#include <chrono>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>
int main()
{
    using Azure::DateTime;
    using Azure::Core::Context;
    using Azure::Core::Credentials::TokenCredentialOptions;
    using Azure::Core::Credentials::TokenRequestContext;
    using Azure::Identity::ManagedIdentityCredential;
    Azure::Core::Diagnostics::Logger::SetLevel(Azure::Core::Diagnostics::Logger::Level::Verbose);

    auto cred = std::make_shared<ManagedIdentityCredential>("foobar");
    static_cast<void>(cred);
    std::cout << "hello";
}
