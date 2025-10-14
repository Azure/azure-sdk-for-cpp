// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Define the test application options.
 *
 */

#pragma once

#include "azure/perf/argagg.hpp"

namespace Azure { namespace Perf {
  /**
   * @brief Define a wrapper container for the test options.
   *
   * @remark This class behaves as wrapper on top of the command line arguments for a test. It help
   * the test to get the parsed options from the command line.
   *
   */
  class TestOptions {
  private:
    argagg::parser_results m_results;

  public:
    /**
     * @brief Create the test options component from the command line parsed results.
     *
     * @param results The command line parsed results.
     */
    explicit TestOptions(argagg::parser_results results) : m_results(results) {}

    /**
     * @brief Get the option value from the option name. If the option is not found, it returns \p
     * defaultValue as default value.
     *
     * @param optionName The name of the option.
     * @param defaultValue A default value to return if the option name was not parsed from command
     * line.
     * @return The option value if it was parsed from command line or the default value.
     */
    template <class T> T GetOptionOrDefault(std::string const& optionName, T defaultValue)
    {
      try
      {
        return m_results[optionName].as<T>();
      }
      catch (argagg::unknown_option const&)
      {
        return defaultValue;
      }
      catch (std::exception const&)
      {
        throw;
      }
      return defaultValue;
    }

    /**
     * @brief Check if the option was parsed from command line.
     *
     * @param optionName The name of the option.
     * @return true if the option was parsed from command line.
     * @return false if the option was not parsed from command line.
     */
    bool HasOption(std::string const& optionName) const
    {
      if (m_results.has_option(optionName))
      {
        // If there are results for this option , but the first result has a nullptr arg, it's a
        // boolean parameter with no arguments so return true.
        if (m_results[optionName].count() != 0)
        {
          // No args were passed for this option, it is present.
          return true;
        }
      }
      return false;
    }

    /**
     * @brief Get the option value from the option name.
     *
     * @remark The option is mandatory.
     *
     * @param optionName The name of the option.
     * @return The option value.
     */
    template <class T> T GetMandatoryOption(std::string const& optionName)
    {
      return m_results[optionName].as<T>();
    }
  };
}} // namespace Azure::Perf
