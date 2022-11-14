//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define the main performance framework program.
 *
 */

#pragma once

#include "azure/perf/argagg.hpp"
#include "azure/perf/test_metadata.hpp"

#include <vector>

namespace Azure { namespace Perf {
  /**
   * @brief Define a performance application.
   *
   */
  class Program {
  private:
    class ArgParser {
    public:
      static argagg::parser_results Parse(
          int argc,
          char** argv,
          std::vector<Azure::Perf::TestOption> const& testOptions);

      static Azure::Perf::GlobalTestOptions Parse(argagg::parser_results const& parsedArgs);
    };

  public:
    /**
     * @brief Start the performance application.
     *
     * @param context The strategy for cancelling the test execution.
     * @param tests The list of tests that the application can run.
     * @param argc The number of command line arguments.
     * @param argv The reference to the first null terminated command line argument.
     */
    static void Run(
        Azure::Core::Context const& context,
        std::vector<Azure::Perf::TestMetadata> const& tests,
        int argc,
        char** argv);
  };
}} // namespace Azure::Perf
