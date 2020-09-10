// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "perf_stress_options.hpp"
#include "perf_stress_test.hpp"

#include <iostream>

using Azure::PerfStress::PerfStressOptions;
using Azure::PerfStress::PerfStressTest;
using std::cout;
using std::endl;

class NoOp : public PerfStressTest {
public:
  NoOp(PerfStressOptions options) : PerfStressTest(options) {}

  void Run(Azure::Core::Context const& ctx) override
  {
    (void)ctx;
    cout << "Hello test" << endl;
  }
};

int main(int argc, char** argv)
{
  (void)argv;
  (void)argc;
  PerfStressOptions op;
  NoOp test(op);
  test.Run(Azure::Core::GetApplicationContext());
  return 0;
}
