// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/perf/argagg.hpp"
#include "azure/perf/program.hpp"

#include <stdexcept>
#include <string>
#include <vector>

#define GET_ARG(Name, Is)

argagg::parser_results Azure::Perf::Program::ArgParser::Parse(
    int argc,
    char** argv,
    std::vector<Azure::Perf::TestOption> const& testOptions)
{
  // Option Name, Activate options, display message and number of expected args.
  argagg::parser argParser;
  auto optionsMetadata = Azure::Perf::GlobalTestOptions::GetOptionMetadata();
  for (auto option : testOptions)
  {
    argParser.definitions.push_back(
        {option.Name, option.Activators, option.DisplayMessage, option.ExpectedArgs});
  }
  for (auto option : optionsMetadata)
  {
    argParser.definitions.push_back(
        {option.Name, option.Activators, option.DisplayMessage, option.ExpectedArgs});
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

Azure::Perf::GlobalTestOptions Azure::Perf::Program::ArgParser::Parse(
    argagg::parser_results const& parsedArgs)
{
  Azure::Perf::GlobalTestOptions options;
  if (parsedArgs["Duration"])
  {
    options.Duration = parsedArgs["Duration"];
  }
  if (parsedArgs["Host"])
  {
    options.Host = parsedArgs["Host"].as<std::string>();
  }
  if (parsedArgs["Insecure"])
  {
    options.Insecure = parsedArgs["Insecure"].as<bool>();
  }
  if (parsedArgs["Iterations"])
  {
    options.Iterations = parsedArgs["Iterations"];
  }
  if (parsedArgs["JobStatistics"])
  {
    options.JobStatistics = parsedArgs["JobStatistics"].as<bool>();
  }
  if (parsedArgs["Latency"])
  {
    options.Latency = parsedArgs["Latency"].as<bool>();
  }
  if (parsedArgs["NoCleanup"])
  {
    options.NoCleanup = parsedArgs["NoCleanup"].as<bool>();
  }
  if (parsedArgs["Parallel"])
  {
    options.Parallel = parsedArgs["Parallel"];
  }
  if (parsedArgs["Port"])
  {
    options.Port = parsedArgs["Port"];
  }
  if (parsedArgs["Rate"])
  {
    options.Rate = parsedArgs["Rate"];
  }
  if (parsedArgs["Warmup"])
  {
    options.Warmup = parsedArgs["Warmup"];
  }
  if (parsedArgs["TestProxies"])
  {
    std::string proxy;
    std::istringstream fullArg(parsedArgs["TestProxies"].as<std::string>());
    while (std::getline(fullArg, proxy, ';'))
    {
      options.TestProxies.push_back(proxy);
    }
  }

  return options;
}
