#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity/default_azure_credential.hpp>

// Uncomment as necessary
// #include <azure/core/http/curl_transport.hpp>
// #include <azure/core/http/win_http_transport.hpp>

#include <chrono>
#include <iostream>
#include <memory>

int main()
{
  // Uncomment the code line below if you want no logging.
  // Otherwise, make sure that you have AZURE_LOG_LEVEL=verbose set in the environment.
  // Azure::Core::Diagnostics::Logger::SetListener(nullptr);

  // Unset all the variables that other credentials read from, to make sure DefaultAzureCredential
  // falls into AzureCliCredential.
  Azure::Core::_internal::Environment::SetVariable("AZURE_AUTHORITY_HOST", "");
  Azure::Core::_internal::Environment::SetVariable("AZURE_CLIENT_CERTIFICATE_PATH", "");
  Azure::Core::_internal::Environment::SetVariable("AZURE_CLIENT_ID", "");
  Azure::Core::_internal::Environment::SetVariable("AZURE_CLIENT_SECRET", "");
  Azure::Core::_internal::Environment::SetVariable("AZURE_FEDERATED_TOKEN_FILE", "");
  Azure::Core::_internal::Environment::SetVariable("AZURE_TENANT_ID", "");
  Azure::Core::_internal::Environment::SetVariable("IDENTITY_ENDPOINT", "");
  Azure::Core::_internal::Environment::SetVariable("IDENTITY_HEADER", "");
  Azure::Core::_internal::Environment::SetVariable("IDENTITY_SERVER_THUMBPRINT", "");
  Azure::Core::_internal::Environment::SetVariable("IMDS_ENDPOINT", "");
  Azure::Core::_internal::Environment::SetVariable("MSI_ENDPOINT", "");
  Azure::Core::_internal::Environment::SetVariable("MSI_SECRET", "");

  Azure::Core::Credentials::TokenCredentialOptions credOptions;

  // Uncomment as necessary. I tested with both - it works.
  //credOptions.Transport.Transport = std::make_shared<Azure::Core::Http::CurlTransport>();
  //credOptions.Transport.Transport = std::make_shared<Azure::Core::Http::WinHttpTransport>();

  const auto cred = std::make_shared<Azure::Identity::DefaultAzureCredential>(credOptions);
  try
  {
    Azure::Core::Credentials::TokenRequestContext trc;
    trc.Scopes.push_back("https://vault.azure.net/.default");
    auto const before = std::chrono::steady_clock::now();
    static_cast<void>(cred->GetToken(trc, Azure::Core::Context::ApplicationContext));
    auto const after = std::chrono::steady_clock::now();
    std::cout << "\n\n-=-=-= Time: "
              << std::chrono::duration_cast<std::chrono::seconds>(after - before).count()
              << " seconds. =-=-=-\n";
  }
  catch (std::exception const& ex)
  {
    std::cout << "\n\n-=-=-= Exception thrown: =-=-=-\n"
              << ex.what() << "\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n";
  }
}