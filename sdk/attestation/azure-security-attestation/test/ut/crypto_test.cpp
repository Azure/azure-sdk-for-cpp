// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/attestation/attestation_client.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <gtest/gtest.h>

#include "..\..\src\private\crypto\inc\crypto.hpp"


namespace Azure { namespace Security { namespace Attestation { namespace Test {
    using namespace Azure::Security::Attestation::_private::Cryptography;

TEST(CryptoTests, CreateRsaKey)
    {
      auto privateKey = Crypto::CreateRsaKey(2048);
      std::string exportedPrivateKey = privateKey->ExportPrivateKey();

      EXPECT_EQ(0, exportedPrivateKey.find("-----BEGIN PRIVATE KEY-----"));

      auto importedKey = Crypto::ImportPrivateKey(exportedPrivateKey);

      std::string exportedPublicKey = privateKey->ExportPublicKey();

      EXPECT_EQ(0, exportedPublicKey.find("-----BEGIN PUBLIC KEY-----"));
      auto importedPublicKey = Crypto::ImportPublicKey(exportedPublicKey);

      EXPECT_THROW(Crypto::ImportPrivateKey(exportedPublicKey), std::runtime_error);
    }
}}}} // namespace Azure::Security::Attestation::Test
