#pragma once
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/internal/serializers.hpp"
#include "azure/data/tables/tables_client.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Data { namespace Test {

  class SerializersTest : public Azure::Storage::Test::StorageTest {
  };
}}} // namespace Azure::Data::Test
