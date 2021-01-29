// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/performance-stress/argagg.hpp"

namespace Azure { namespace PerformanceStress {
  class TestOptions {
  private:
    argagg::parser_results m_results;

  public:
    TestOptions(argagg::parser_results results) : m_results(results) {}

    template <class T> T GetOptionOrDefault(std::string const& option, T def)
    {
      if (m_results[option])
      {
        return m_results[option].as<T>();
      }
      return def;
    }
  };
}} // namespace Azure::PerformanceStress
