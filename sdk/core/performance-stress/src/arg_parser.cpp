// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/performance-stress/argagg.hpp"
#include "azure/performance-stress/program.hpp"

#include <stdexcept>

Azure::PerformanceStress::Options Azure::PerformanceStress::Program::ArgParser::Parse(
    int argc,
    char** argv,
    std::string& testName)
{
  // Option Name, Activate options, display message and number of expected args.
  argagg::parser argParser{{{"help", {"-h", "--h"}, "Display help information.", 0}}};

  argagg::parser_results args;
  // Will throw on fail
  args = argParser.parse(argc, argv);

  if (args["help"])
  {
    std::cerr << argParser;
    std::exit(0);
  }

  if (args.pos.size() != 1)
  {
    throw std::runtime_error("Mising test name or multiple test name as input");
  }

  testName = std::string(args.pos[0]);

  // Init default test options
  Azure::PerformanceStress::Options options;
  return options;
}
