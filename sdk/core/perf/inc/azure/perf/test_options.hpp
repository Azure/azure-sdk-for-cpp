// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define the properties of a test option.
 *
 */

#pragma once

#include <azure/core/context.hpp>

#include <string>
#include <vector>

namespace Azure { namespace Perf {
  /**
   * @brief Define the properties of a test option that can be parsed from command line.
   *
   */
  struct TestOption
  {
    /**
     * @brief The name of the test option.
     *
     */
    std::string Name;
    /**
     * @brief The list of sentinels for parsing the option from command line. i. e. [`-o`,
     * `--option`].
     *
     */
    std::vector<std::string> Activators;
    /**
     * @brief The message that is displayed in the command line when help is requested.
     *
     */
    std::string DisplayMessage;
    /**
     * @brief The number of arguments expected after the sentinel for the test option.
     *
     */
    uint16_t ExpectedArgs;

    /**
     * @brief Make an option to be mandatory to run the test.
     *
     */
    bool Required = false;

    /**
     * @brief Make the option to be replaced with **** on all outputs
     *
     */
    bool SensitiveData = false;
  };
}} // namespace Azure::Perf
