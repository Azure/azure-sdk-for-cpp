// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/attestation/attestation_client.hpp>

using namespace Azure::Security::Attestation;

TEST(Attestation, Basic)
{
  AttestationClient attestationClient;

  EXPECT_FALSE(attestationClient.ClientVersion().empty());
}

TEST(Attestation, GetValue)
{
  AttestationClient attestationClient;

  EXPECT_EQ(attestationClient.GetValue(-1), 0);
  EXPECT_EQ(attestationClient.GetValue(0), 1);
  EXPECT_EQ(attestationClient.GetValue(1), 2);
}
