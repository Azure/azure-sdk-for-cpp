// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"

#include <azure/keyvault/common/internal/base64url.hpp>
#include <azure/keyvault/key_vault.hpp>

#include "key_client_base_test.hpp"

#include <string>
#include <thread>

using namespace Azure::Security::KeyVault::Keys::Test;
using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Common::_internal;

TEST_F(KeyVaultClientTest, ImportKey)
{
  KeyClient keyClient(m_keyVaultUrl, m_credential);
  JsonWebKey key;
  key.KeyType = JsonWebKeyType::Rsa;
  // Values from https://docs.microsoft.com/en-us/rest/api/keyvault/importkey/importkey
  key.N = Base64Url::Base64UrlDecode(
      "nKAwarTrOpzd1hhH4cQNdVTgRF-b0ubPD8ZNVf0UXjb62QuAk3Dn68ESThcF7SoDYRx2QVcfoMC9WCcuQUQDieJF-"
      "lvJTSer1TwH72NBovwKlHvrXqEI0a6_uVYY5n-"
      "soGt7qFZNbwQLdWWA6PrbqTLIkv6r01dcuhTiQQAn6OWEa0JbFvWfF1kILQIaSBBBaaQ4R7hZs7-"
      "VQTHGD7J1xGteof4gw2VTiwNdcE8p5UG5b6S9KQwAeET4yB4KFPwQ3TDdzxJQ89mwYVi_"
      "sgAIggN54hTq4oEKYJHBOMtFGIN0_HQ60ZSUnpOi87xNC-8VFqnv4rfTQ7nkK6XMvjMVfw");
  key.E = Base64Url::Base64UrlDecode("AQAB");
  key.D = Base64Url::Base64UrlDecode("GeT1_D5LAZa7qlC7WZ0DKJnOth8kcPrN0urTEFtWCbmHQWkAad_px_"
                                     "VUpGp0BWDDzENbXbQcu4QCCdf4crve5eXt8dVI86OSah");
  key.DP = Base64Url::Base64UrlDecode(
      "ZGnmWx-Nca71z9a9vvT4g02iv3S-"
      "3kSgmhl8JST09YQwK8tfiK7nXnNMtXJi2K4dLKKnLicGtCzB6W3mXdLcP2SUOWDOeStoBt8HEBT"
      "4MrI1psCKqnBum78WkHju90rBFj99amkP6UeQy5EASAzgmKQu2nUaUnRV0lYP8LHMCkE");
  key.DQ = Base64Url::Base64UrlDecode(
      "dtpke0foFs04hPS6XYLA5lc7-1MAHfZKN4CkMAofwDqPmRQzCxpDJUk0gMWGJEdU_"
      "Lqfbg22Py44cci0dczH36NW3UU5BL86T2_SPPDOuyX7kDscrIJCdowxQCGJHGRBEozM_"
      "uTL46wu6UnUIv7m7cuGgodJyZBcdwpo6ziFink");
  key.QI = Base64Url::Base64UrlDecode(
      "Y9KD5GaHkAYmAqpOfAQUMr71QuAAaBb0APzMuUvoEYw39PD3_vJeh9HZ15QmJ8zCX10-"
      "nlzUB-bWwvK-rGcJXbK4pArilr5MiaYv7e8h5eW2zs2_itDJ6Oebi-"
      "wVbMhg7DvUTBbkCvPhhIedE4UlDQmMYP7RhzVVs7SfmkGs_DQ");
  key.P = Base64Url::Base64UrlDecode(
      "v1jeCPnuJQM2PW2690Q9KJk0Ulok8VFGjkcHUHVi3orKdy7y_"
      "TCIWM6ZGvgFzI6abinzYbTEPKV4wFdMAwvOWmawXj5YrsoeB44_HXJ0ak_5_"
      "iP6XXR8MLGXbd0ZqsxvAZyzMj9vyle7EN2cBod6aenI2QZoRDucPvjPwZsZotk");
  key.Q = Base64Url::Base64UrlDecode(
      "0Yv-Dj6qnvx_LL70lUnKA6MgHE_bUC4drl5ZNDDsUdUUYfxIK4G1rGU45kHGtp-Qg-"
      "Uyf9s52ywLylhcVE3jfbjOgEozlSwKyhqfXkLpMLWHqOKj9fcfYd4PWKPOgpzWsqjA6fJbBUM"
      "Yo0CU2G9cWCtVodO7sBJVSIZunWrAlBc");
  std::string keyName(GetUniqueName());
  auto response = keyClient.ImportKey(keyName, key);
  CheckValidResponse(response);

  {
    // delete + purge
    auto op = keyClient.StartDeleteKey(keyName);
    op.PollUntilDone(std::chrono::milliseconds(1000));
    keyClient.PurgeDeletedKey(keyName);
  }
}
