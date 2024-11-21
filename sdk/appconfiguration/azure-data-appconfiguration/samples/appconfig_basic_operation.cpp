// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample provides the code implementation to use the App Configuration client SDK for
 * C++ to get one or more configuration settings.
 */

#include <azure/data/appconfiguration.hpp>
#include <azure/identity.hpp>

#include <iostream>

using namespace Azure::Data::AppConfiguration;
using namespace Azure::Identity;

int main()
{
  try
  {
    std::string url = "https://<your-appconfig-name>.azconfig.io";
    auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();

    // create client
    ConfigurationClient configurationClient(url, credential);

    Azure::Response<GetKeyValueResult> response = configurationClient.GetKeyValue("key", "accept");

    std::cout << response.Value.Value.Value() << std::endl;
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication error:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Client request failed error:" << std::endl << e.what() << std::endl;
    return 1;
  }

  return 0;
}
