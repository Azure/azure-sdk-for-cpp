// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

class EventHubsStressScenario {

public:
  EventHubsStressScenario(){};
  virtual void Warmup(int repetitions) = 0;
  virtual void Run(int repetitions) = 0;
  virtual void Cleanup() = 0;

protected:
  ~EventHubsStressScenario(){};
};
