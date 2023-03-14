// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/azure_cli_credential.hpp"

#include "private/token_credential_impl.hpp"

#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/core/internal/unique_handle.hpp>
#include <azure/core/platform.hpp>

#include <cstdio>
#include <locale>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

using Azure::Identity::AzureCliCredential;

using Azure::DateTime;
using Azure::Core::Context;
using Azure::Core::_internal::Environment;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_internal::Log;
using Azure::Identity::AzureCliCredentialOptions;
using Azure::Identity::_detail::TokenCache;
using Azure::Identity::_detail::TokenCredentialImpl;

namespace {
constexpr auto IdentityPrefix = "Identity: ";
}

void AzureCliCredential::ThrowIfNotSafeCmdLineInput(
    std::string const& input,
    std::string const& description) const
{
  for (auto const c : input)
  {
    switch (c)
    {
      case ':':
      case '/':
      case '.':
      case '-':
      case '_':
      case ' ':
        break;

      default:
        if (!std::isalnum(c, std::locale::classic()))
        {
          throw AuthenticationException(
              IdentityPrefix + GetCredentialName() + ": Unsafe command line input found in "
              + description + ": " + input);
        }
    }
  }
}
AzureCliCredential::AzureCliCredential(
    std::string tenantId,
    DateTime::duration cliProcessTimeout,
    Core::Credentials::TokenCredentialOptions const& options)
    : TokenCredential("AzureCliCredential"), m_tenantId(std::move(tenantId)),
      m_cliProcessTimeout(std::move(cliProcessTimeout))
{
  static_cast<void>(options);

  ThrowIfNotSafeCmdLineInput(m_tenantId, "TenantID");

  auto const logLevel = Logger::Level::Informational;
  if (Log::ShouldWrite(logLevel))
  {
    Log::Write(
        logLevel,
        IdentityPrefix + GetCredentialName()
            + " created.\n"
              "Successful creation does not guarantee further successful token retrieval.");
  }
}

AzureCliCredential::AzureCliCredential(AzureCliCredentialOptions const& options)
    : AzureCliCredential(options.TenantId, options.CliProcessTimeout, options)
{
}

AzureCliCredential::AzureCliCredential(TokenCredentialOptions const& options)
    : AzureCliCredential(
        AzureCliCredentialOptions{}.TenantId,
        AzureCliCredentialOptions{}.CliProcessTimeout,
        options)
{
}

std::string AzureCliCredential::GetAzCommand(std::string const& scopes, std::string const& tenantId)
    const
{
  ThrowIfNotSafeCmdLineInput(scopes, "Scopes");
  std::string command = "az account get-access-token --output json --scope \"" + scopes + "\"";

  if (!tenantId.empty())
  {
    command += " --tenant \"" + tenantId + "\"";
  }

  return command;
}

namespace {
std::string RunShellCommand(
    std::string const& command,
    DateTime::duration timeout,
    Context const& context);
}

AccessToken AzureCliCredential::GetToken(
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  auto const scopes = TokenCredentialImpl::FormatScopes(tokenRequestContext.Scopes, false, false);

  // TokenCache::GetToken() can only use the lambda argument when they are being executed. They are
  // not supposed to keep a reference to lambda argument to call it later. Therefore, any capture
  // made here will outlive the possible time frame when the lambda might get called.
  return m_tokenCache.GetToken(scopes, tokenRequestContext.MinimumExpiration, [&]() {
    try
    {
      auto const azCliResult
          = RunShellCommand(GetAzCommand(scopes, m_tenantId), m_cliProcessTimeout, context);

      try
      {
        return TokenCredentialImpl::ParseToken(
            azCliResult, "accessToken", "expiresIn", "expiresOn");
      }
      catch (std::exception const&)
      {
        // Throw the az command output (error message)
        // limited to 250 characters (250 has no special meaning).
        throw std::runtime_error(azCliResult.substr(0, 250));
      }
    }
    catch (std::exception const& e)
    {
      auto const errorMsg
          = IdentityPrefix + GetCredentialName() + " didn't get the token: \"" + e.what() + '\"';

      auto const logLevel = Logger::Level::Warning;
      if (Log::ShouldWrite(logLevel))
      {
        Log::Write(logLevel, errorMsg);
      }

      throw AuthenticationException(errorMsg);
    }
  });
}

namespace {
#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP // not UWP
template <typename> struct UniqueHandleHelper;
template <> struct UniqueHandleHelper<HANDLE>
{
  static void CloseWin32Handle(HANDLE handle) { static_cast<void>(CloseHandle(handle)); }
  using type = Azure::Core::_internal::BasicUniqueHandle<void, CloseWin32Handle>;
};

template <typename T>
using UniqueHandle = Azure::Core::_internal::UniqueHandle<T, UniqueHandleHelper>;
#endif // not UWP
#endif

class ShellProcess;
class OutputPipe final {
  friend class ShellProcess;

private:
#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP // not UWP
  UniqueHandle<HANDLE> m_writeHandle;
  UniqueHandle<HANDLE> m_readHandle;
  OVERLAPPED m_overlapped = {};
#endif // not UWP
#else // not Windows
  std::vector<int> m_fd;
#endif

  OutputPipe(OutputPipe const&) = delete;
  OutputPipe& operator=(OutputPipe const&) = delete;

public:
  OutputPipe();

  ~OutputPipe();

  bool NonBlockingRead(
      std::vector<char>& buffer,
      std::remove_reference<decltype(buffer)>::type::size_type& bytesRead,
      bool& willHaveMoreData);
};

class ShellProcess final {
private:
#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP // not UWP
  UniqueHandle<HANDLE> m_processHandle;
#endif // not UWP
#else // not Windows
  std::vector<char*> m_argv;
  std::vector<char> m_argvValues;

  std::vector<char*> m_envp;
  std::vector<char> m_envpValues;

  posix_spawn_file_actions_t m_actions = {};
  pid_t m_pid = -1;
#endif

  ShellProcess(ShellProcess const&) = delete;
  ShellProcess& operator=(ShellProcess const&) = delete;

  void Finalize();

public:
  ShellProcess(std::string const& command, OutputPipe& outputPipe);
  ~ShellProcess() { Finalize(); }

  void Terminate();
};

std::string RunShellCommand(
    std::string const& command,
    DateTime::duration timeout,
    Context const& context)
{
  // Use steady_clock so we're not affected by system time rewinding.
  auto const terminateAfter = std::chrono::steady_clock::now()
      + std::chrono::duration_cast<std::chrono::steady_clock::duration>(timeout);

  std::string output;

  OutputPipe pipe;
  ShellProcess shellProcess(command, pipe);

  // Typically token json is just a bit less than 2KiB.
  // The best buffer size is the one that lets us to read it in one go.
  // (Should it be smaller, we will succeed as well, it'll just take more iterations).
  std::vector<char> processOutputBuf(2 * 1024);

  auto willHaveMoreData = true;
  do
  {
    // Check if we should terminate
    {
      if (context.IsCancelled())
      {
        shellProcess.Terminate();
        throw std::runtime_error("Context was cancelled before Azure CLI process was done.");
      }

      if (std::chrono::steady_clock::now() > terminateAfter)
      {
        shellProcess.Terminate();
        throw std::runtime_error("Azure CLI process took too long to complete.");
      }
    }

    decltype(processOutputBuf)::size_type bytesRead = 0;
    if (pipe.NonBlockingRead(processOutputBuf, bytesRead, willHaveMoreData))
    {
      output.insert(output.size(), processOutputBuf.data(), bytesRead);
    }
    else if (willHaveMoreData)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Value has no special meaning.
    }
  } while (willHaveMoreData);

  return output;
}

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP // not UWP
void ThrowIfApiCallFails(BOOL apiResult, std::string const& errMsg)
{
  // LCOV_EXCL_START
  if (!apiResult)
  {
    throw std::runtime_error(
        errMsg + ": " + std::to_string(GetLastError())

    );
  }
  // LCOV_EXCL_STOP
}
#endif // not UWP
#else // not Windows
void ThrowIfApiCallFails(int apiResult, std::string const& errMsg)
{
  // LCOV_EXCL_START
  if (apiResult != 0)
  {
    throw std::runtime_error(
        errMsg + ": " + std::to_string(apiResult) + " (errno: " + std::to_string(errno) + ")");
  }
  // LCOV_EXCL_STOP
}
#endif

OutputPipe::OutputPipe()
{
#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP // not UWP
  SECURITY_ATTRIBUTES pipeSecurity = {};
  pipeSecurity.nLength = sizeof(decltype(pipeSecurity));
  pipeSecurity.bInheritHandle = TRUE;
  pipeSecurity.lpSecurityDescriptor = nullptr;

  {
    HANDLE readHandle = nullptr;
    HANDLE writeHandle = nullptr;

    ThrowIfApiCallFails(
        CreatePipe(&readHandle, &writeHandle, &pipeSecurity, 0), "Cannot create output pipe");

    m_readHandle.reset(readHandle);
    m_writeHandle.reset(writeHandle);
  }

  ThrowIfApiCallFails(
      SetHandleInformation(m_readHandle.get(), HANDLE_FLAG_INHERIT, 0),
      "Cannot ensure the read handle for the output pipe is not inherited");
#else // UWP
  throw std::runtime_error("The credential is not supported on UWP.");
#endif
#else // not Windows
  m_fd.push_back(-1);
  m_fd.push_back(-1);

  ThrowIfApiCallFails(pipe(m_fd.data()), "Cannot create output pipe");
  ThrowIfApiCallFails(
      fcntl(m_fd[0], F_SETFL, O_NONBLOCK), "Cannot set up output pipe to have non-blocking read");
#endif
}

OutputPipe::~OutputPipe()
{
#if !defined(AZ_PLATFORM_WINDOWS)
  for (auto iter = m_fd.rbegin(); iter != m_fd.rend(); ++iter)
  {
    if (*iter != -1)
    {
      static_cast<void>(close(*iter));
    }
  }
#endif
}

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP // not UWP
void AppendToEnvironmentValuesIfNotEmpty(
    std::vector<CHAR>& environmentValues,
    std::string const& envVarName,
    std::string const& value)
{
  if (!value.empty()) // LCOV_EXCL_LINE
  {
    auto const envVarStatement = envVarName + "=" + value;

    environmentValues.insert(
        environmentValues.end(), envVarStatement.begin(), envVarStatement.end());

    environmentValues.push_back('\0'); // terminate the string
  }
}

void AppendToEnvironmentValuesIfDefined(
    std::vector<CHAR>& environmentValues,
    std::string const& envVarName)
{
  AppendToEnvironmentValuesIfNotEmpty(
      environmentValues, envVarName, Environment::GetVariable(envVarName.c_str()));
}
#endif // not UWP
#else // not Windows
void AppendToArgvValues(
    std::vector<char>& argvValues,
    std::vector<std::remove_reference<decltype(argvValues)>::type::size_type>& argvValuePositions,
    std::string const& value)
{
  argvValuePositions.push_back(argvValues.size());
  argvValues.insert(argvValues.end(), value.begin(), value.end());
  argvValues.push_back('\0');
}

void EnsureShellExists(std::string const& pathToShell)
{
  auto file = std::fopen(pathToShell.c_str(), "r");

  // LCOV_EXCL_START
  if (!file)
  {
    throw std::runtime_error("Cannot locate command line shell.");
  }
  // LCOV_EXCL_STOP

  std::fclose(file);
}
#endif

ShellProcess::ShellProcess(std::string const& command, OutputPipe& outputPipe)
{
#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP // not UWP
  // Start the process.
  PROCESS_INFORMATION procInfo = {};

  {
    STARTUPINFO startupInfo = {};
    startupInfo.cb = sizeof(decltype(startupInfo));
    startupInfo.dwFlags |= STARTF_USESTDHANDLES; // cspell:disable-line
    startupInfo.hStdInput = INVALID_HANDLE_VALUE;
    startupInfo.hStdOutput = outputPipe.m_writeHandle.get();
    startupInfo.hStdError = outputPipe.m_writeHandle.get();

    // Path to cmd.exe
    std::vector<CHAR> commandLineStr;
    {
      auto const commandLine = "cmd /c " + command;
      commandLineStr.insert(commandLineStr.end(), commandLine.begin(), commandLine.end());
      commandLineStr.push_back('\0');
    }

    // Form the environment
    std::vector<CHAR> environmentValues;
    LPVOID lpEnvironment = nullptr;
    {
      {
        constexpr auto PathEnvVarName = "PATH";
        auto pathValue = Environment::GetVariable(PathEnvVarName);

        for (auto const pf :
             {Environment::GetVariable("ProgramFiles"),
              Environment::GetVariable("ProgramFiles(x86)")})
        {
          if (!pf.empty()) // LCOV_EXCL_LINE
          {
            if (!pathValue.empty()) // LCOV_EXCL_LINE
            {
              pathValue += ";";
            }

            pathValue += pf + "\\Microsoft SDKs\\Azure\\CLI2\\wbin";
          }
        }

        AppendToEnvironmentValuesIfNotEmpty(environmentValues, PathEnvVarName, pathValue);
      }

      // Also provide SystemRoot variable.
      // Without it, 'az' may fail with the following error:
      // "Fatal Python error: _Py_HashRandomization_Init: failed to get random numbers to
      // initialize Python
      // Python runtime state: preinitialized
      // ".
      AppendToEnvironmentValuesIfDefined(environmentValues, "SystemRoot");

      // Also provide USERPROFILE variable.
      // Without it, we'll be getting "ERROR: Please run 'az login' to setup account." even if the
      // user did log in.
      AppendToEnvironmentValuesIfDefined(environmentValues, "USERPROFILE");

      if (!environmentValues.empty()) // LCOV_EXCL_LINE
      {
        environmentValues.push_back('\0'); // terminate the block
        lpEnvironment = environmentValues.data();
      }
    }

    ThrowIfApiCallFails(
        CreateProcessA(
            nullptr,
            commandLineStr.data(),
            nullptr,
            nullptr,
            TRUE,
            NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
            lpEnvironment,
            nullptr,
            &startupInfo,
            &procInfo),
        "Cannot create process");
  }

  // We won't be needing the process main thread handle on our end.
  static_cast<void>(CloseHandle(procInfo.hThread));

  // Keep the process handle so we can cancel it if it takes too long.
  m_processHandle.reset(procInfo.hProcess);

  // We won't be writing to the pipe that is meant for the process.
  // We will only be reading the pipe.
  // So, now that the process is started, we can close write handle on our end.
  outputPipe.m_writeHandle.reset();
#else // UWP
  static_cast<void>(command);
  static_cast<void>(outputPipe);
#endif // UWP
#else // not Windows
  // Form the 'argv' array:
  // * An array of pointers to non-const C strings (0-terminated).
  // * Last element is nullptr.
  // * First element (at index 0) is path to a program.
  {
    // Since the strings that argv is pointing at do need to be non-const,
    // and also because each commnd line argument needs to be a separate 0-terminated string,
    // We do form all their values in the m_argvValues.

    // Since we append m_argvValues as we go, at one point after insertion it may reallocate the
    // buffer to a different address in memory. For that reason, we can't grab addresses before we
    // are done forming m_argvValues contents - so until that we record indices where each string
    // start - in argvValuePositions.
    {
      std::vector<decltype(m_argvValues)::size_type> argvValuePositions;

      // First string is the path to executable, and not the actual first argument.
      {
        std::string const Shell = "/bin/sh";
        EnsureShellExists(Shell);
        AppendToArgvValues(m_argvValues, argvValuePositions, Shell);
      }

      // Second argument is the shell switch that tells the command line shell to execute a command
      AppendToArgvValues(m_argvValues, argvValuePositions, "-c");

      // Third value is the command that needs to be executed.
      AppendToArgvValues(m_argvValues, argvValuePositions, command);

      // We are done appending to m_argvValues, so it is now safe to grab addresses to the elements
      // in it.
      for (auto const pos : argvValuePositions)
      {
        m_argv.push_back(m_argvValues.data() + pos);
      }
    }

    // argv last element needs to be nullptr.
    m_argv.push_back(nullptr);
  }

  // Form the 'envp' array:
  // * An array of pointers to non-const C strings (0-terminated).
  // * Strings are in form key=value (PATH uses ':' as separator)
  // * Last element is nullptr.
  // * First element (at index 0) is path to a program.
  {
    auto const actualPathVarValue = Environment::GetVariable("PATH");
    auto const processPathVarStatement = std::string("PATH=") + actualPathVarValue
        + (actualPathVarValue.empty() ? "" : ":") + "/usr/bin:/usr/local/bin";

    m_envpValues.insert(
        m_envpValues.end(), processPathVarStatement.begin(), processPathVarStatement.end());

    m_envpValues.push_back('\0');

    // We should only grab m_envpValues.data() as we're done appending to it, because appends may
    // reallocate the buffer to a different memory location.
    m_envp.push_back(m_envpValues.data());
    m_envp.push_back(nullptr);
  }

  // Set up pipe communication for the process.
  static_cast<void>(posix_spawn_file_actions_init(&m_actions));
  static_cast<void>(posix_spawn_file_actions_addclose(&m_actions, outputPipe.m_fd[0]));
  static_cast<void>(posix_spawn_file_actions_adddup2(&m_actions, outputPipe.m_fd[1], 1));
  static_cast<void>(posix_spawn_file_actions_addclose(&m_actions, outputPipe.m_fd[1]));

  {
    auto const spawnResult
        = posix_spawn(&m_pid, m_argv[0], &m_actions, NULL, m_argv.data(), m_envp.data());

    // LCOV_EXCL_START
    if (spawnResult != 0)
    {
      m_pid = -1;
      Finalize();
      ThrowIfApiCallFails(spawnResult, "Cannot spawn process");
    }
    // LCOV_EXCL_STOP
  }

  close(outputPipe.m_fd[1]);
  outputPipe.m_fd[1] = -1;
#endif
}

void ShellProcess::Finalize()
{
#if !defined(AZ_PLATFORM_WINDOWS)
  if (m_pid > 0)
  {
    static_cast<void>(waitpid(m_pid, nullptr, 0));
  }

  posix_spawn_file_actions_destroy(&m_actions);
#endif
}

void ShellProcess::Terminate()
{
#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP // not UWP
  static_cast<void>(TerminateProcess(m_processHandle.get(), 0));
#endif // not UWP
#else // not Windows
  if (m_pid > 0)
  {
    static_cast<void>(kill(m_pid, SIGKILL));
  }
#endif
}

bool OutputPipe::NonBlockingRead(
    std::vector<char>& buffer,
    std::remove_reference<decltype(buffer)>::type::size_type& bytesRead,
    bool& willHaveMoreData)
{
#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP // not UWP
  static_assert(
      sizeof(std::remove_reference<decltype(buffer)>::type::value_type) == sizeof(CHAR),
      "buffer elements and CHARs should be of the same size");

  // Since we're using OVERLAPPED, call to ReadFile() is non-blocking - ReadFile() would return
  // immediately if there is no data, and won't wait for any data to arrive.
  DWORD bytesReadDword = 0;
  auto const hadData
      = (ReadFile(
             m_readHandle.get(),
             buffer.data(),
             static_cast<DWORD>(buffer.size()),
             &bytesReadDword,
             &m_overlapped)
         == TRUE);

  bytesRead = static_cast<std::remove_reference<decltype(bytesRead)>::type>(bytesReadDword);

  // Invoking code should be calling this function until we set willHaveMoreData to true.
  // We set it to true when we receive ERROR_BROKEN_PIPE after ReadFile(), which means the process
  // has finished and closed the pipe on its end, and it means there won't be more data after
  // what've just read.
  willHaveMoreData = (GetLastError() != ERROR_BROKEN_PIPE);

  return hadData && bytesRead > 0;
#else // UWP
  static_cast<void>(buffer);
  static_cast<void>(bytesRead);
  static_cast<void>(willHaveMoreData);
  throw std::runtime_error("The credential is not supported on UWP.");
#endif // UWP
#else // not Windows
  static_assert(
      sizeof(std::remove_reference<decltype(buffer)>::type::value_type) == sizeof(char),
      "buffer elements and chars should be of the same size");

  auto const nread = read(m_fd[0], buffer.data(), static_cast<size_t>(buffer.size()));

  bytesRead = static_cast<std::remove_reference<decltype(bytesRead)>::type>(nread < 0 ? 0 : nread);
  willHaveMoreData = (nread > 0 || (nread == -1 && errno == EAGAIN));
  return nread > 0;
#endif
}
} // namespace
