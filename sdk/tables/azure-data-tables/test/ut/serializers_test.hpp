#pragma once
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test/ut/test_base.hpp"

#include <azure/data/tables/serializers.hpp>
#include <azure/data/tables/tables_clients.hpp>

namespace Azure { namespace Data { namespace Test {

  class SerializersTest : public Azure::Storage::Test::StorageTest {
  };
}}} // namespace Azure::Data::Test
