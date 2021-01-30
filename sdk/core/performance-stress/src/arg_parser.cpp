// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/performance-stress/argagg.hpp"
#include "azure/performance-stress/program.hpp"

#include <stdexcept>
#include <vector>

argagg::parser_results Azure::PerformanceStress::Program::ArgParser::Parse(
    int argc,
    char** argv,
    std::vector<Azure::PerformanceStress::TestOption> const& testOptions)
{
  // Option Name, Activate options, display message and number of expected args.
  argagg::parser argParser;
  auto optionsMetadata = Azure::PerformanceStress::GlobalTestOptions::GetOptionMetadata();
  for (auto option : testOptions)
  {
    argParser.definitions.push_back(
        {option.Name, option.Activators, option.DisplayMessage, option.expectedArgs});
  }
  for (auto option : optionsMetadata)
  {
    argParser.definitions.push_back(
        {option.Name, option.Activators, option.DisplayMessage, option.expectedArgs});
  }

  // Will throw on fail
  auto argsResults = argParser.parse(argc, argv);

  if (argsResults["help"])
  {
    std::cerr << argParser;
    std::exit(0);
  }

  if (argsResults.pos.size() != 1)
  {
    throw std::runtime_error("Mising test name or multiple test name as input");
  }

  return argsResults;
}

Azure::PerformanceStress::GlobalTestOptions Azure::PerformanceStress::Program::ArgParser::Parse(
    argagg::parser_results const& parsedArgs)
{
  Azure::PerformanceStress::GlobalTestOptions options;
  if (parsedArgs["Duration"])
  {
    options.Duration = parsedArgs["Duration"];
  }
  if (parsedArgs["Host"])
  {
    options.Host = parsedArgs["Host"].as<std::string>();
  }
  if (parsedArgs["Parallel"])
  {
    options.Parallel = parsedArgs["Parallel"];
  }
  if (parsedArgs["Iterations"])
  {
    options.Iterations = parsedArgs["Iterations"];
  }

  return options;
}
