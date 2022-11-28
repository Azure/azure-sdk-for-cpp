// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/process_helper.hpp"

#include <azure/core/platform.hpp>

#include <chrono>
#include <stdexcept>
#include <thread>
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
#include <array>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

using Azure::Identity::_detail::ProcessHelper;

using Azure::DateTime;
using Azure::Core::Context;

namespace {
class Process;
class OutputPipe {
  friend class Process;

private:
#if defined(AZ_PLATFORM_WINDOWS)
  HANDLE m_writeHandle = NULL;
  HANDLE m_readHandle = NULL;
  OVERLAPPED m_overlapped = {};
#else
  std::array<int, 2> m_fd = {-1, -1};
#endif

  OutputPipe(OutputPipe const&) = delete;
  OutputPipe& operator=(OutputPipe const&) = delete;

public:
  OutputPipe();

  ~OutputPipe();

  bool NonBlockingRead(
      std::vector<std::string::value_type>& buffer,
      std::string::size_type& bytesRead,
      bool& willHaveMoreData);
};

class Process {
private:
#if defined(AZ_PLATFORM_WINDOWS)
  HANDLE m_processHandle = NULL;
#else
  int m_pid = -1;
  posix_spawn_file_actions_t m_actions = {};

  std::vector<char*> m_argv;
  std::vector<char> m_argvValues;
#endif

  Process(Process const&) = delete;
  Process& operator=(Process const&) = delete;

public:
  Process(std::string const& executable, std::string const& arguments, OutputPipe& outputPipe);
  ~Process();

  void Terminate();
};
} // namespace

std::string ProcessHelper::ExecuteProcess(
    std::string const& executable,
    std::string const& arguments,
    Context const& context,
    DateTime::duration timeout)
{
  // Use steady_clock so we're not affected by system time rewinding.
  auto const terminateAfter = std::chrono::steady_clock::now()
      + std::chrono::duration_cast<std::chrono::steady_clock::duration>(timeout);

  std::string output;

  OutputPipe pipe;
  Process process(executable, arguments, pipe);

  // Typically token json is just a bit less than 2KiB.
  // The best buffer size is the one that lets us to read it in one go.
  // (Should it be smaller, we will succeed as well, it'll just take more iterations).
  std::vector<std::string::value_type> processOutputBuf(2 * 1024);

  auto willHaveMoreData = true;
  do
  {
    // Check if we should terminate
    {
      if (context.IsCancelled())
      {
        process.Terminate();
        throw std::runtime_error("Context was canceled before Azure CLI process was done.");
      }

      if (std::chrono::steady_clock::now() > terminateAfter)
      {
        process.Terminate();
        throw std::runtime_error("Azure CLI process took too long to complete.");
      }
    }

    std::string::size_type bytesRead = 0;
    if (pipe.NonBlockingRead(processOutputBuf, bytesRead, willHaveMoreData))
    {
      output.insert(output.size(), processOutputBuf.data(), bytesRead);
    }
    else if (willHaveMoreData)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Value has no special meaning.
    }
  } while (willHaveMoreData);

  return output;
}

namespace {
OutputPipe::OutputPipe()
{
#if defined(AZ_PLATFORM_WINDOWS)
  SECURITY_ATTRIBUTES pipeSecurity = {};
  ZeroMemory(&pipeSecurity, sizeof(decltype(pipeSecurity)));
  pipeSecurity.nLength = sizeof(decltype(pipeSecurity));
  pipeSecurity.bInheritHandle = TRUE;
  pipeSecurity.lpSecurityDescriptor = NULL;

  auto const createPipeResult = CreatePipe(&m_readHandle, &m_writeHandle, &pipeSecurity, 0);
  // LCOV_EXCL_START
  if (!createPipeResult)
  {
    throw std::runtime_error("Cannot create output pipe.");
  }
  // LCOV_EXCL_STOP

  auto const setHandleInfoResult = SetHandleInformation(m_readHandle, HANDLE_FLAG_INHERIT, 0);
  // LCOV_EXCL_START
  if (!setHandleInfoResult)
  {
    throw std::runtime_error("Cannot ensure the read handle for the output pipe is not inherited.");
  }
  // LCOV_EXCL_STOP

  // We use OVERLAPPED when we ReadFile(), so that the call is non-blocking, which lets us to also
  // poll whether we should terminate the process.
  ZeroMemory(&m_overlapped, sizeof(decltype(m_overlapped)));
#else
  {
    auto const pipeResult = pipe(m_fd.data());
    // LCOV_EXCL_START
    if (pipeResult != 0)
    {
      throw std::runtime_error("Cannot create output pipe.");
    }
    // LCOV_EXCL_STOP
  }

  {
    auto const fcntlResult = fcntl(m_fd[0], F_SETFL, O_NONBLOCK);
    // LCOV_EXCL_START
    if (fcntlResult != 0)
    {
      this->~OutputPipe();
      throw std::runtime_error("Cannot set up output pipe to have non-blocking read.");
    }
    // LCOV_EXCL_STOP
  }
#endif
}

OutputPipe::~OutputPipe()
{
#if defined(AZ_PLATFORM_WINDOWS)
  if (m_writeHandle != NULL)
  {
    static_cast<void>(CloseHandle(m_writeHandle));
  }

  if (m_readHandle != NULL)
  {
    static_cast<void>(CloseHandle(m_readHandle));
  }
#else
  for (auto iter = m_fd.rbegin(); iter != m_fd.rend(); ++iter)
  {
    if (*iter != -1)
    {
      static_cast<void>(close(*iter));
    }
  }
#endif
}

Process::Process(
    std::string const& executable,
    std::string const& arguments,
    OutputPipe& outputPipe)
{
#if defined(AZ_PLATFORM_WINDOWS)
  // Start the process.
  PROCESS_INFORMATION procInfo = {};
  ZeroMemory(&procInfo, sizeof(decltype(procInfo)));

  {
    STARTUPINFO startupInfo = {};
    ZeroMemory(&startupInfo, sizeof(decltype(startupInfo)));
    startupInfo.cb = sizeof(decltype(startupInfo));
    startupInfo.hStdOutput = outputPipe.m_writeHandle;
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;
    startupInfo.hStdError = INVALID_HANDLE_VALUE;
    startupInfo.hStdInput = INVALID_HANDLE_VALUE;

    auto commandLine = executable + " " + arguments;

    auto commandLineStr = std::vector<CHAR>(commandLine.begin(), commandLine.end());
    commandLineStr.push_back('\0');

    if (!CreateProcessA(
            NULL,
            commandLineStr.data(),
            NULL,
            NULL,
            TRUE,
            NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
            NULL,
            NULL,
            &startupInfo,
            &procInfo))
    {
      throw std::runtime_error("Cannot create process.");
    }
  }

  // We won't be needing the process main thread handle on our end.
  static_cast<void>(CloseHandle(procInfo.hThread));

  // Keep the process handle so we can cancel it if it takes too long.
  m_processHandle = procInfo.hProcess;

  // We won't be writing to the pipe that is meant for the process.
  // We will only be reading the pipe.
  // So, now that the process is started, we can close write handle on our end.
  static_cast<void>(CloseHandle(outputPipe.m_writeHandle));
  outputPipe.m_writeHandle = NULL;
#else
  posix_spawn_file_actions_init(&m_actions);
  posix_spawn_file_actions_addclose(&m_actions, outputPipe.m_fd[0]);
  posix_spawn_file_actions_adddup2(&m_actions, outputPipe.m_fd[1], 1);
  posix_spawn_file_actions_addclose(&m_actions, outputPipe.m_fd[1]);

  // Form the 'argv' array:
  // * An array of pointers to non-const C strings (0-terminated).
  // * Last element is NULL pointer.
  // * First element (at index 0) is path to a program.
  {
    // Since the strings that argv is pointing at do need to be non-const,
    // and also because each commnd line argument needs to be a separate 0-terminated string,
    // We do form all their values in the m_argvValues.

    // Since we append m_argvValues as we go, at one point after insertion it may reallocate the
    // buffer to a different address in memory. For that reason, we can't grab addresses before we
    // are done forming m_argvValues contents - so until that we record indices where each string
    // start - in argvValuePositions.
    std::vector<decltype(m_argvValues)::size_type> argvValuePositions;
    argvValuePositions.push_back(0); // First string starts at index (offset) 0.

    // First string is the path to executable, and not the actual first argument.
    m_argvValues.insert(m_argvValues.begin(), executable.begin(), executable.end());
    m_argvValues.push_back('\0');

    // Now go through the arguments.
    {
      // We split arguments by space character. A fully-functional implementation needs to be able
      // to ignore some of the spaces, if they are inside quote characters, and these quote
      // characters were not escaped. But, for using az cli we don't need any of that.
      // We could've need it for testing, so that we can execute commands like
      // `/bin/bash -c "ping 127.0.0.1"`,
      // but apparently we can work around it using the TAB character: just write arguments as
      // `"-c ping\t127.0.0.1"`.

      // newArgStart is `true` on the first character of each argument.
      auto newArgStart = true;
      for (auto c : arguments)
      {
        if (c != ' ')
        {
          if (newArgStart)
          {
            argvValuePositions.push_back(m_argvValues.size());
            newArgStart = false;
          }

          m_argvValues.push_back(c);
        }
        else if (!newArgStart)
        {
          m_argvValues.push_back('\0');
          newArgStart = true;
          // multiple spaces in a row will be treated as one.
        }
      }

      // Unless arguments end with the space character, we did not 0-terminate it. But we need to.
      if (!newArgStart)
      {
        m_argvValues.push_back('\0');
      }
    }

    // We are done appending to m_argvValues, so it is now safe to grab addresses to the elements in
    // it.
    for (auto const pos : argvValuePositions)
    {
      m_argv.push_back(m_argvValues.data() + pos);
    }

    // argv last element needs to be NULL pointer.
    m_argv.push_back(NULL);
  }

  // Note: unlike Win32 API's CreateProcessA(), posix_spawn() does not return error if executable
  // does not exist, and then it is more complicated to distinguish between successful and zombie
  // processes. The test program I ran with some isolated code from this file did result in
  // "free(): double free detected in tcache 2" and Aborted. But luckily AzureCliCredential is
  // supposed to check for the binary to exist.
  auto const spawnResult = posix_spawn(&m_pid, m_argv[0], &m_actions, NULL, m_argv.data(), NULL);

  // LCOV_EXCL_START
  if (spawnResult != 0)
  {
    m_pid = -1;
    this->~Process();
  }
  // LCOV_EXCL_STOP

  close(outputPipe.m_fd[1]);
  outputPipe.m_fd[1] = -1;
#endif
}

Process::~Process()
{
#if defined(AZ_PLATFORM_WINDOWS)
  static_cast<void>(CloseHandle(m_processHandle));
#else
  if (m_pid > 0)
  {
    static_cast<void>(waitpid(m_pid, NULL, 0));
  }

  posix_spawn_file_actions_destroy(&m_actions);
#endif
}

void Process::Terminate()
{
#if defined(AZ_PLATFORM_WINDOWS)
  static_cast<void>(TerminateProcess(m_processHandle, 0));
#else
  if (m_pid > 0)
  {
    static_cast<void>(kill(m_pid, SIGKILL));
  }
#endif
}

bool OutputPipe::NonBlockingRead(
    std::vector<std::string::value_type>& buffer,
    std::string::size_type& bytesRead,
    bool& willHaveMoreData)
{
#if defined(AZ_PLATFORM_WINDOWS)
  static_assert(
      sizeof(std::remove_reference<decltype(buffer)>::type::value_type) == sizeof(CHAR),
      "buffer elements and CHARs should be of the same size");

  // Since we're using OVERLAPPED, call to ReadFile() is non-blocking - ReadFile() would return
  // immediately if there is no data, and won;t wait for any data to arrive.
  DWORD bytesReadDword = 0;
  auto const hadData
      = (ReadFile(
             m_readHandle,
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
#else
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
