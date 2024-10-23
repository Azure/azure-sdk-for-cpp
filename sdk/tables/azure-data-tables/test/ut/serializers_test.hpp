#pragma once
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/private/serializers.hpp"
#include "azure/data/tables/tables_clients.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Data { namespace Test {

  class SerializersTest : public Azure::Storage::Test::StorageTest {};
}}} // namespace Azure::Data::Test
