// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/http/transport.hpp>
#include <azure/identity.hpp>
#include <azure/storage/files/datalake.hpp>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using Azure::Storage::Files::DataLake::DataLakeFileSystemClient;

struct CommandLineOptions final
{
  bool ValidateOnly = false;
  bool Mock = false;
  std::string ManifestPath;
  std::size_t RequiredCases = 0;
  std::string OutputPath;
};

struct ManifestRow final
{
  std::size_t LineNumber = 0;
  std::string Cloud;
  std::string Ring;
  std::string ApiFamily;
  std::string ServiceUrl;
  std::string WorkspaceId;
  std::string ProbePath;
  std::string ParseError;
};

struct CaseResult final
{
  std::size_t LineNumber = 0;
  std::string Cloud;
  std::string Ring;
  std::string ApiFamily;
  std::string Status;
  std::string Message;
};

struct Summary final
{
  std::size_t Required = 0;
  std::size_t Executed = 0;
  std::size_t Passed = 0;
  std::size_t Failed = 0;
  std::size_t Skipped = 0;
};

class ValidationCredential final : public Azure::Core::Credentials::TokenCredential {
public:
  ValidationCredential() : TokenCredential("ValidationCredential") {}

  Azure::Core::Credentials::AccessToken GetToken(
      Azure::Core::Credentials::TokenRequestContext const&,
      Azure::Core::Context const&) const override
  {
    throw std::runtime_error("manifest validation must not request a token");
  }
};

class MockCredential final : public Azure::Core::Credentials::TokenCredential {
public:
  MockCredential() : TokenCredential("MockCredential") {}

  Azure::Core::Credentials::AccessToken GetToken(
      Azure::Core::Credentials::TokenRequestContext const&,
      Azure::Core::Context const&) const override
  {
    Azure::Core::Credentials::AccessToken token;
    token.Token = "mock-token";
    token.ExpiresOn = Azure::DateTime::clock::now() + std::chrono::hours(1);
    return token;
  }
};

class MockProbeComplete final : public std::exception {
public:
  const char* what() const noexcept override { return "mock probe reached transport"; }
};

class RecordingTransport final : public Azure::Core::Http::HttpTransport {
public:
  std::vector<std::string> RequestHosts;

  std::unique_ptr<Azure::Core::Http::RawResponse> Send(
      Azure::Core::Http::Request& request,
      Azure::Core::Context const&) override
  {
    RequestHosts.emplace_back(request.GetUrl().GetHost());
    throw MockProbeComplete();
  }
};

bool StartsWith(const std::string& value, const std::string& prefix)
{
  return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

std::size_t ParsePositiveSize(const std::string& value, const std::string& optionName)
{
  if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
  {
    throw std::invalid_argument(optionName + " requires a positive integer");
  }
  const auto parsed = std::stoul(value);
  if (parsed == 0)
  {
    throw std::invalid_argument(optionName + " requires a positive integer");
  }
  return static_cast<std::size_t>(parsed);
}

std::string ReadOptionValue(
    int& index,
    int argumentCount,
    char** arguments,
    const std::string& optionName)
{
  if (index + 1 >= argumentCount)
  {
    throw std::invalid_argument(optionName + " requires a value");
  }
  return arguments[++index];
}

CommandLineOptions ParseCommandLine(int argumentCount, char** arguments)
{
  CommandLineOptions options;
  for (int index = 1; index < argumentCount; ++index)
  {
    const std::string argument(arguments[index]);
    if (argument == "--validate-only")
    {
      options.ValidateOnly = true;
    }
    else if (argument == "--mock")
    {
      options.Mock = true;
    }
    else if (argument == "--manifest")
    {
      options.ManifestPath = ReadOptionValue(index, argumentCount, arguments, "--manifest");
    }
    else if (StartsWith(argument, "--manifest="))
    {
      options.ManifestPath = argument.substr(std::string("--manifest=").size());
    }
    else if (argument == "--require-cases")
    {
      options.RequiredCases = ParsePositiveSize(
          ReadOptionValue(index, argumentCount, arguments, "--require-cases"), "--require-cases");
    }
    else if (StartsWith(argument, "--require-cases="))
    {
      options.RequiredCases = ParsePositiveSize(
          argument.substr(std::string("--require-cases=").size()), "--require-cases");
    }
    else if (argument == "--output")
    {
      options.OutputPath = ReadOptionValue(index, argumentCount, arguments, "--output");
    }
    else if (StartsWith(argument, "--output="))
    {
      options.OutputPath = argument.substr(std::string("--output=").size());
    }
    else
    {
      throw std::invalid_argument("Unknown argument: " + argument);
    }
  }

  if (options.ManifestPath.empty())
  {
    const char* manifestPath = std::getenv("ONELAKE_WORKSPACE_MATRIX_TSV");
    if (manifestPath != nullptr)
    {
      options.ManifestPath = manifestPath;
    }
  }
  if (options.ManifestPath.empty())
  {
    throw std::invalid_argument("--manifest or ONELAKE_WORKSPACE_MATRIX_TSV is required");
  }
  if (options.RequiredCases == 0)
  {
    throw std::invalid_argument("--require-cases is required");
  }
  if (options.OutputPath.empty())
  {
    throw std::invalid_argument("--output is required");
  }
  if (options.ValidateOnly && options.Mock)
  {
    throw std::invalid_argument("--validate-only and --mock are mutually exclusive");
  }
  if (!options.ValidateOnly && !options.Mock)
  {
    const char* testMode = std::getenv("AZURE_TEST_MODE");
    if (testMode == nullptr || std::string(testMode) != "LIVE")
    {
      throw std::invalid_argument("Live probes require AZURE_TEST_MODE=LIVE");
    }
    if (options.RequiredCases != 32)
    {
      throw std::invalid_argument("Live probes require --require-cases=32");
    }
  }
  return options;
}

std::vector<std::string> SplitTabs(const std::string& line)
{
  std::vector<std::string> fields;
  std::size_t begin = 0;
  while (true)
  {
    const auto end = line.find('\t', begin);
    fields.emplace_back(line.substr(begin, end - begin));
    if (end == std::string::npos)
    {
      break;
    }
    begin = end + 1;
  }
  return fields;
}

bool IsValidUtf8(const std::string& value)
{
  for (std::size_t index = 0; index < value.size();)
  {
    const auto firstByte = static_cast<unsigned char>(value[index]);
    if (firstByte <= 0x7f)
    {
      ++index;
      continue;
    }

    std::size_t continuationBytes = 0;
    std::uint32_t codePoint = 0;
    if (firstByte >= 0xc2 && firstByte <= 0xdf)
    {
      continuationBytes = 1;
      codePoint = firstByte & 0x1f;
    }
    else if (firstByte >= 0xe0 && firstByte <= 0xef)
    {
      continuationBytes = 2;
      codePoint = firstByte & 0x0f;
    }
    else if (firstByte >= 0xf0 && firstByte <= 0xf4)
    {
      continuationBytes = 3;
      codePoint = firstByte & 0x07;
    }
    else
    {
      return false;
    }
    if (index + continuationBytes >= value.size())
    {
      return false;
    }
    for (std::size_t offset = 1; offset <= continuationBytes; ++offset)
    {
      const auto continuationByte = static_cast<unsigned char>(value[index + offset]);
      if (continuationByte < 0x80 || continuationByte > 0xbf)
      {
        return false;
      }
      codePoint = (codePoint << 6) | (continuationByte & 0x3f);
    }
    if ((continuationBytes == 2 && codePoint < 0x800)
        || (continuationBytes == 3 && codePoint < 0x10000)
        || (codePoint >= 0xd800 && codePoint <= 0xdfff) || codePoint > 0x10ffff)
    {
      return false;
    }
    index += continuationBytes + 1;
  }
  return true;
}

std::vector<ManifestRow> ReadManifest(const std::string& manifestPath)
{
  std::ifstream manifest(manifestPath);
  if (!manifest)
  {
    throw std::runtime_error("Cannot open manifest: " + manifestPath);
  }

  std::vector<ManifestRow> rows;
  std::string line;
  std::size_t lineNumber = 0;
  while (std::getline(manifest, line))
  {
    ++lineNumber;
    if (!line.empty() && line.back() == '\r')
    {
      line.pop_back();
    }
    if (line.empty() || line[0] == '#')
    {
      continue;
    }

    const auto fields = SplitTabs(line);
    ManifestRow row;
    row.LineNumber = lineNumber;
    if (!fields.empty())
    {
      row.Cloud = fields[0];
    }
    if (fields.size() > 1)
    {
      row.Ring = fields[1];
    }
    if (fields.size() > 2)
    {
      row.ApiFamily = fields[2];
    }
    if (fields.size() > 3)
    {
      row.ServiceUrl = fields[3];
    }
    if (fields.size() > 4)
    {
      row.WorkspaceId = fields[4];
    }
    if (fields.size() > 5)
    {
      row.ProbePath = fields[5];
    }
    if (fields.size() != 6)
    {
      row.ParseError = "expected 6 tab-separated columns, found " + std::to_string(fields.size());
    }
    rows.emplace_back(std::move(row));
  }
  return rows;
}

template <std::size_t Size>
bool Contains(const std::array<const char*, Size>& values, const std::string& value)
{
  for (const auto candidate : values)
  {
    if (value == candidate)
    {
      return true;
    }
  }
  return false;
}

const std::array<const char*, 4> CloudDomains{
    "fabric.microsoft.com",
    "fabric-df.microsoft.com",
    "fabric.microsoft.us",
    "fabric.sovcloud-api.fr"};
const std::array<const char*, 4> Rings{"base", "daily", "dxt", "msit"};
const std::array<const char*, 2> ApiFamilies{"dfs", "blob"};

bool HasValidUtf8(const ManifestRow& row)
{
  return IsValidUtf8(row.Cloud) && IsValidUtf8(row.Ring) && IsValidUtf8(row.ApiFamily)
      && IsValidUtf8(row.ServiceUrl) && IsValidUtf8(row.WorkspaceId) && IsValidUtf8(row.ProbePath);
}

template <std::size_t Size>
std::string GetSafeLabel(
    const std::array<const char*, Size>& allowedValues,
    const std::string& value)
{
  return IsValidUtf8(value) && Contains(allowedValues, value) ? value : "<invalid>";
}

std::string GetCaseKey(const ManifestRow& row)
{
  return row.Cloud + "\t" + row.Ring + "\t" + row.ApiFamily;
}

std::set<std::string> GetExpectedMatrixKeys()
{
  std::set<std::string> keys;
  for (const auto cloud : CloudDomains)
  {
    for (const auto ring : Rings)
    {
      for (const auto apiFamily : ApiFamilies)
      {
        keys.emplace(std::string(cloud) + "\t" + ring + "\t" + apiFamily);
      }
    }
  }
  return keys;
}

std::string CompactWorkspaceId(const std::string& workspaceId)
{
  std::string compact;
  for (auto character : workspaceId)
  {
    if (character == '-')
    {
      continue;
    }
    if (character >= 'A' && character <= 'Z')
    {
      character = static_cast<char>(character - 'A' + 'a');
    }
    compact.push_back(character);
  }
  return compact;
}

std::string GetExpectedHost(const ManifestRow& row, const std::string& apiFamily)
{
  const auto compactWorkspaceId = CompactWorkspaceId(row.WorkspaceId);
  const std::string ringPrefix = row.Ring == "base" ? "" : row.Ring + "-";
  return compactWorkspaceId + ".z" + compactWorkspaceId.substr(0, 2) + "." + ringPrefix + apiFamily
      + "." + row.Cloud;
}

CaseResult MakeResult(const ManifestRow& row, const std::string& status, const std::string& message)
{
  CaseResult result;
  result.LineNumber = row.LineNumber;
  result.Cloud = GetSafeLabel(CloudDomains, row.Cloud);
  result.Ring = GetSafeLabel(Rings, row.Ring);
  result.ApiFamily = GetSafeLabel(ApiFamilies, row.ApiFamily);
  result.Status = status;
  result.Message = message;
  return result;
}

DataLakeFileSystemClient ValidateEndpoint(
    const ManifestRow& row,
    const std::shared_ptr<const Azure::Core::Credentials::TokenCredential>& credential)
{
  auto client = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
      row.ServiceUrl, row.WorkspaceId, credential);
  const auto actualBlobHost = Azure::Core::Url(client.GetUrl()).GetHost();
  const auto expectedBlobHost = GetExpectedHost(row, "blob");
  if (actualBlobHost != expectedBlobHost)
  {
    throw std::runtime_error(
        "manifest labels resolve to " + expectedBlobHost + ", not " + actualBlobHost);
  }
  return client;
}

void SendProbe(const ManifestRow& row, const DataLakeFileSystemClient& client)
{
  if (row.ApiFamily == "dfs")
  {
    auto response = client.GetDirectoryClient(row.ProbePath).ListPaths(false);
    static_cast<void>(response);
  }
  else
  {
    auto response = client.GetFileClient(row.ProbePath).GetProperties();
    static_cast<void>(response);
  }
}

void RunLiveProbe(
    const ManifestRow& row,
    const std::shared_ptr<const Azure::Core::Credentials::TokenCredential>& credential)
{
  auto client = ValidateEndpoint(row, credential);
  SendProbe(row, client);
}

void RunMockProbe(const ManifestRow& row)
{
  auto credential = std::make_shared<MockCredential>();
  auto transport = std::make_shared<RecordingTransport>();
  Azure::Storage::Files::DataLake::DataLakeClientOptions options;
  options.Retry.MaxRetries = 0;
  options.Transport.Transport = transport;
  auto client = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
      row.ServiceUrl, row.WorkspaceId, credential, options);

  bool reachedTransport = false;
  try
  {
    SendProbe(row, client);
  }
  catch (const MockProbeComplete&)
  {
    reachedTransport = true;
  }
  if (!reachedTransport || transport->RequestHosts.size() != 1)
  {
    throw std::runtime_error("mock probe did not make exactly one request");
  }
  const auto expectedHost = GetExpectedHost(row, row.ApiFamily);
  if (transport->RequestHosts[0] != expectedHost)
  {
    throw std::runtime_error(
        "mock probe targeted " + transport->RequestHosts[0] + ", not " + expectedHost);
  }
}

std::string EscapeJson(const std::string& value)
{
  if (!IsValidUtf8(value))
  {
    throw std::invalid_argument("Invalid UTF-8 in JSON evidence");
  }

  const char* hexDigits = "0123456789abcdef";
  std::string escaped;
  for (const auto rawCharacter : value)
  {
    const auto character = static_cast<unsigned char>(rawCharacter);
    switch (character)
    {
      case '\\':
        escaped += "\\\\";
        break;
      case '"':
        escaped += "\\\"";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      default:
        if (character < 0x20)
        {
          escaped += "\\u00";
          escaped += hexDigits[(character >> 4) & 0x0f];
          escaped += hexDigits[character & 0x0f];
        }
        else
        {
          escaped += static_cast<char>(character);
        }
        break;
    }
  }
  return escaped;
}

std::string GetExecutionMode(const CommandLineOptions& options)
{
  if (options.ValidateOnly)
  {
    return "validate";
  }
  return options.Mock ? "mock" : "live";
}

std::string SerializeResults(
    const std::string& executionMode,
    std::size_t probeAttempts,
    const Summary& summary,
    const std::vector<CaseResult>& results,
    const std::vector<std::string>& matrixErrors)
{
  std::ostringstream output;

  output << "{\n  \"mode\": \"" << executionMode << "\",\n  \"probeAttempts\": " << probeAttempts
         << ",\n  \"summary\": {\"required\": " << summary.Required
         << ", \"executed\": " << summary.Executed << ", \"passed\": " << summary.Passed
         << ", \"failed\": " << summary.Failed << ", \"skipped\": " << summary.Skipped
         << "},\n  \"results\": [";
  for (std::size_t index = 0; index < results.size(); ++index)
  {
    const auto& result = results[index];
    output << (index == 0 ? "\n" : ",\n") << "    {\"line\": " << result.LineNumber
           << ", \"cloud\": \"" << EscapeJson(result.Cloud) << "\", \"ring\": \""
           << EscapeJson(result.Ring) << "\", \"apiFamily\": \"" << EscapeJson(result.ApiFamily)
           << "\", \"status\": \"" << EscapeJson(result.Status) << "\", \"message\": \""
           << EscapeJson(result.Message) << "\"}";
  }
  output << (results.empty() ? "" : "\n  ") << "],\n  \"matrixErrors\": [";
  for (std::size_t index = 0; index < matrixErrors.size(); ++index)
  {
    output << (index == 0 ? "" : ", ") << "\"" << EscapeJson(matrixErrors[index]) << "\"";
  }
  output << "]\n}\n";
  if (!output)
  {
    throw std::runtime_error("Cannot serialize result evidence");
  }
  return output.str();
}

void WriteResults(const std::string& outputPath, const std::string& serializedResults)
{
  std::ofstream output(outputPath, std::ios::binary);
  if (!output)
  {
    throw std::runtime_error("Cannot open output file: " + outputPath);
  }
  output.write(serializedResults.data(), static_cast<std::streamsize>(serializedResults.size()));
  output.flush();
  if (!output)
  {
    throw std::runtime_error("Cannot write output file: " + outputPath);
  }
}

Summary GetSummary(std::size_t requiredCases, const std::vector<CaseResult>& results)
{
  Summary summary;
  summary.Required = requiredCases;
  summary.Executed = results.size();
  for (const auto& result : results)
  {
    if (result.Status == "passed")
    {
      ++summary.Passed;
    }
    else if (result.Status == "failed")
    {
      ++summary.Failed;
    }
    else if (result.Status == "skipped")
    {
      ++summary.Skipped;
    }
  }
  return summary;
}

int Run(const CommandLineOptions& options)
{
  const auto rows = ReadManifest(options.ManifestPath);
  std::vector<CaseResult> results;
  std::vector<std::string> matrixErrors;
  std::set<std::string> observedKeys;
  std::size_t probeAttempts = 0;

  bool manifestHasValidUtf8 = true;
  for (const auto& row : rows)
  {
    manifestHasValidUtf8 = manifestHasValidUtf8 && HasValidUtf8(row);
  }

  if (!manifestHasValidUtf8)
  {
    for (const auto& row : rows)
    {
      results.emplace_back(MakeResult(
          row,
          "failed",
          HasValidUtf8(row) ? "manifest preflight failed" : "manifest field is not valid UTF-8"));
    }
  }
  else
  {
    auto validationCredential = std::make_shared<ValidationCredential>();
    for (const auto& row : rows)
    {
      if (!row.ParseError.empty())
      {
        results.emplace_back(MakeResult(row, "failed", row.ParseError));
        continue;
      }
      if (!Contains(CloudDomains, row.Cloud) || !Contains(Rings, row.Ring)
          || !Contains(ApiFamilies, row.ApiFamily))
      {
        results.emplace_back(MakeResult(row, "failed", "unsupported matrix label"));
        continue;
      }

      const auto key = GetCaseKey(row);
      if (!observedKeys.insert(key).second)
      {
        results.emplace_back(MakeResult(row, "failed", "duplicate matrix key"));
        continue;
      }
      if (row.ProbePath.empty())
      {
        results.emplace_back(MakeResult(row, "skipped", "probe path is empty"));
        continue;
      }
      if (row.ProbePath.front() == '/' || row.ProbePath.find_first_of("?#") != std::string::npos
          || row.ProbePath.find("://") != std::string::npos)
      {
        results.emplace_back(MakeResult(row, "failed", "probe path must be relative"));
        continue;
      }

      try
      {
        ValidateEndpoint(row, validationCredential);
        results.emplace_back(MakeResult(row, "passed", "validated"));
      }
      catch (const std::exception&)
      {
        results.emplace_back(MakeResult(row, "failed", "endpoint validation failed"));
      }
    }
  }

  if (rows.size() != options.RequiredCases)
  {
    matrixErrors.emplace_back(
        "manifest contains " + std::to_string(rows.size()) + " rows; expected "
        + std::to_string(options.RequiredCases));
  }
  const auto expectedKeys = GetExpectedMatrixKeys();
  if (options.RequiredCases == expectedKeys.size() && observedKeys != expectedKeys)
  {
    matrixErrors.emplace_back("manifest does not contain the complete 32-case matrix");
  }

  auto summary = GetSummary(options.RequiredCases, results);
  const bool manifestIsValid = summary.Failed == 0 && summary.Skipped == 0
      && summary.Passed == summary.Required && matrixErrors.empty();
  if (!options.ValidateOnly && manifestIsValid)
  {
    std::shared_ptr<const Azure::Core::Credentials::TokenCredential> liveCredential;
    if (!options.Mock)
    {
      liveCredential = std::make_shared<Azure::Identity::DefaultAzureCredential>();
    }
    for (std::size_t index = 0; index < rows.size(); ++index)
    {
      ++probeAttempts;
      try
      {
        if (options.Mock)
        {
          RunMockProbe(rows[index]);
        }
        else
        {
          RunLiveProbe(rows[index], liveCredential);
        }
        results[index].Message = options.Mock ? "mock probe succeeded" : "probe succeeded";
      }
      catch (const std::exception&)
      {
        results[index].Status = "failed";
        results[index].Message = "probe failed";
      }
    }
    summary = GetSummary(options.RequiredCases, results);
  }

  const auto serializedResults
      = SerializeResults(GetExecutionMode(options), probeAttempts, summary, results, matrixErrors);
  WriteResults(options.OutputPath, serializedResults);

  for (const auto& result : results)
  {
    std::cout << result.Cloud << '/' << result.Ring << '/' << result.ApiFamily << ": "
              << result.Status << std::endl;
  }
  for (const auto& matrixError : matrixErrors)
  {
    std::cerr << matrixError << std::endl;
  }
  return summary.Failed == 0 && summary.Skipped == 0 && summary.Passed == summary.Required
          && matrixErrors.empty()
      ? 0
      : 1;
}
} // namespace

int main(int argumentCount, char** arguments)
{
  try
  {
    return Run(ParseCommandLine(argumentCount, arguments));
  }
  catch (const std::exception& exception)
  {
    std::cerr << exception.what() << std::endl;
    return 2;
  }
}