// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "argagg.hpp"

#include <cstdint>

constexpr const char* EventHubsLoggerName = "eventhubs_stress_test";

extern bool LogToConsole;

struct EventHubsScenarioOptions
{
  /**
   * @brief The name of the scenario option.
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
  std::string HelpMessage;
  /**
   * @brief The number of arguments expected after the sentinel for the test option.
   *
   */
  std::uint16_t ExpectedArgs;

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

class EventHubsStressScenario {
public:
  EventHubsStressScenario() {};
  virtual const std::string& GetStressScenarioName() = 0;
  virtual const std::vector<EventHubsScenarioOptions>& GetScenarioOptions() = 0;
  virtual void Initialize(argagg::parser_results const& parserResults) = 0;
  virtual void Run() = 0;
  virtual void Cleanup() = 0;

protected:
  virtual ~EventHubsStressScenario() {};
};
