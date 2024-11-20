#pragma once
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/account_sas_builder.hpp"
#include "azure/data/tables/tables_sas_builder.hpp"
#include "test_base.hpp"

namespace Azure { namespace Data { namespace Test {

  class SasTest : public Azure::Core::Test::TestBase {
  public:
    static std::map<std::string, std::string> ParseQueryParameters(const std::string& query)
    {
      std::map<std::string, std::string> result;

      auto parameters = Azure::Core::_internal::StringExtensions::Split(query, '&');
      for (const auto& p : parameters)
      {
        auto keyValue = Azure::Core::_internal::StringExtensions::Split(p, '=');
        if (keyValue.size() == 2)
        {
          result[keyValue[0]] = keyValue[1];
        }
      }

      return result;
    }
  };
}}} // namespace Azure::Data::Test
