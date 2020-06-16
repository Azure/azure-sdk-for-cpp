// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "gtest/gtest.h"

namespace Azure  {  namespace Storage  {  namespace Test  {

struct TestUtility 
 {
  constexpr static const char* k_STANDARD_STORAGE_CONNECTION_STRING = "";
  constexpr static const char* k_PREMIUM_STORAGE_CONNECTION_STRING = "";
  constexpr static const char* k_BLOB_STORAGE_CONNECTION_STRING = "";
  constexpr static const char* k_PREMIUM_FILE_CONNECTION_STRING = "";
  constexpr static const char* k_ADLS_GEN2_CONNECTION_STRING = "";
  // TODO: Temporary
  constexpr static const char* k_ACCOUNT_SAS = "";
};

}}}
