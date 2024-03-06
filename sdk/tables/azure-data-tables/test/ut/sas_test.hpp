#pragma once
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/account_sas_builder.hpp"
#include "azure/data/tables/table_sas_builder.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Data { namespace Test {

  class SasTest : public Azure::Core::Test::TestBase {
  public:
    static std::vector<std::string> SplitString(const std::string& s, char separator)
    {
      std::vector<std::string> result;

      const auto len = s.size();
      size_t start = 0;
      while (start < len)
      {
        auto end = s.find(separator, start);
        if (end == std::string::npos)
        {
          end = len;
        }

        result.push_back(s.substr(start, end - start));

        start = end + 1;
      }

      return result;
    }

    static std::map<std::string, std::string> ParseQueryParameters(const std::string& query)
    {
      std::map<std::string, std::string> result;

      auto parameters = SplitString(query, '&');
      for (const auto& p : parameters)
      {
        auto keyValue = SplitString(p, '=');
        if (keyValue.size() == 2)
        {
          result[keyValue[0]] = keyValue[1];
        }
      }

      return result;
    }
  };
}}} // namespace Azure::Data::Test
