// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once


constexpr const char * EventHubsLoggerName = "eventhubs_stress_test";

class EventHubsStressScenario {

public:
  EventHubsStressScenario(){};
  virtual const std::string& GetStressScenarioName() = 0;
  virtual void Initialize() = 0;
  virtual void Warmup(int repetitions) = 0;
  virtual void Run(int repetitions) = 0;
  virtual void Cleanup() = 0;

protected:
  ~EventHubsStressScenario(){};
};
