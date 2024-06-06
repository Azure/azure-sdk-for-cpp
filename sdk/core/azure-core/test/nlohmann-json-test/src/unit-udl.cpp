//     __ _____ _____ _____
//  __|  |   __|     |   | |  JSON for Modern C++ (supporting code)
// |  |  |__   |  |  | | | |  version 3.11.3
// |_____|_____|_____|_|___|  https://github.com/nlohmann/json
//
// SPDX-FileCopyrightText: 2013-2023 Niels Lohmann <https://nlohmann.me>
// SPDX-License-Identifier: MIT

#include "doctest_compatibility.h"

#include <azure/core/internal/json/json.hpp>

TEST_CASE("user-defined string literals")
{
  auto j_expected = Azure::Core::Json::_internal::json::parse(R"({"foo": "bar", "baz": 42})");
  auto ptr_expected = Azure::Core::Json::_internal::json::json_pointer("/foo/bar");

  SECTION("using namespace Azure::Core::Json::_internal::literals::json_literals")
  {
    using namespace Azure::Core::Json::_internal::literals::
        json_literals; // NOLINT(google-build-using-namespace)

    CHECK(R"({"foo": "bar", "baz": 42})"_json == j_expected);
    CHECK("/foo/bar"_json_pointer == ptr_expected);
  }

  SECTION("using namespace Azure::Core::Json::_internal::json_literals")
  {
    using namespace Azure::Core::Json::_internal::
        json_literals; // NOLINT(google-build-using-namespace)

    CHECK(R"({"foo": "bar", "baz": 42})"_json == j_expected);
    CHECK("/foo/bar"_json_pointer == ptr_expected);
  }

  SECTION("using namespace Azure::Core::Json::_internal::literals")
  {
    using namespace Azure::Core::Json::_internal::literals; // NOLINT(google-build-using-namespace)

    CHECK(R"({"foo": "bar", "baz": 42})"_json == j_expected);
    CHECK("/foo/bar"_json_pointer == ptr_expected);
  }

  SECTION("using namespace Azure::Core::Json::_internal")
  {
    using namespace Azure::Core::Json::_internal; // NOLINT(google-build-using-namespace)

    CHECK(R"({"foo": "bar", "baz": 42})"_json == j_expected);
    CHECK("/foo/bar"_json_pointer == ptr_expected);
  }

#ifndef JSON_TEST_NO_GLOBAL_UDLS
  SECTION("global namespace")
  {
    CHECK(R"({"foo": "bar", "baz": 42})"_json == j_expected);
    CHECK("/foo/bar"_json_pointer == ptr_expected);
  }
#endif
}
