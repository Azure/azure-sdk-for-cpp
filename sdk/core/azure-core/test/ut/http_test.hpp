// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief The base class for the HTTP test cases.
 *
 */

#include <azure/core/http/http.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Test {

  class TestHttp : public ::testing::Test {
  };

}}} // namespace Azure::Core::Test
