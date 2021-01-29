// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/performance-stress/argagg.hpp"
#include "azure/performance-stress/program.hpp"

Azure::PerformanceStress::Options Azure::PerformanceStress::Program::ArgParser::Parse(
    int argc,
    char** argv)
{
  // Option Name, Activate options, display message and number of expected args.
  argagg::parser argParser{
      {{"testName", {"-t", "--testName"}, "Name of the test to run", 0},
       {"help", {"-h", "--h"}, "Display help information.", 0}}};

  argagg::parser_results args;
  // Will throw on fail
  args = argParser.parse(argc, argv);

  if (args["help"])
  {
    std::cerr << argParser;
    std::terminate();
  }
  if (args.count() == 0)
  {
    std::cerr << "No arguments provided" << std::endl << argParser;
    std::abort();
  }

  // Init default test options
  Azure::PerformanceStress::Options options;
  return options;
}
