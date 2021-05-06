// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/common/storage_common.hpp>

#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST(MetadataTest, CaseInsensitive)
  {
    const std::string value1 = "vAlUe1";
    const std::string value2 = "vaLue2";

    Metadata metadata;
    metadata["KEY"] = value1;
    EXPECT_EQ(metadata["KEY"], value1);
    EXPECT_EQ(metadata["key"], value1);
    metadata["key"] = value2;
    EXPECT_EQ(metadata["KEY"], value2);
    EXPECT_EQ(metadata["key"], value2);
    EXPECT_EQ(metadata["KeY"], value2);
    EXPECT_EQ(metadata["kEy"], value2);
  }

}}} // namespace Azure::Storage::Test
