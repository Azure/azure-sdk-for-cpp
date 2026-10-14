// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_WINDOWS) && defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/win_http_transport.hpp"
#include "azure/core/io/body_stream.hpp"
#include "azure/core/url.hpp"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <future>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Test {

  namespace {

    // How long to wait for the transport call to return before declaring the thread stuck. The
    // call either succeeds or throws within milliseconds, so any wait beyond a few seconds means
    // the destructor is blocked.
    constexpr auto TransportCallTimeout = std::chrono::seconds(30);

    // The URL is never contacted: WinHttpOpen(), WinHttpConnect() and WinHttpOpenRequest() only
    // allocate handles, and the request fails during setup before anything is sent on the wire.
    constexpr const char* TestUrl = "https://localhost/";

    // A body stream whose Length() throws.
    //
    // WinHttpRequest::SendRequest() calls request.GetBodyStream()->Length() before it calls
    // WinHttpSendRequest(), so throwing from Length() aborts the request after the WinHttpRequest
    // has been constructed (its status callback is registered) but before WinHttpSendRequest()
    // associates the request context with the handle.
    class ThrowingLengthBodyStream final : public Azure::Core::IO::BodyStream {
    public:
      int64_t Length() const override
      {
        throw std::runtime_error("Injected failure from BodyStream::Length");
      }

    private:
      size_t OnRead(uint8_t*, size_t, Azure::Core::Context const&) override { return 0; }
    };

    void SendRequestThatFailsDuringSetup()
    {
      Azure::Core::Http::WinHttpTransport transport;
      ThrowingLengthBodyStream bodyStream;
      Azure::Core::Http::Request request(
          Azure::Core::Http::HttpMethod::Put, Azure::Core::Url(TestUrl), &bodyStream);

      Azure::Core::Context context;
      static_cast<void>(transport.Send(request, context));
    }

  } // namespace

  // A request that fails during setup, before WinHttpSendRequest() associates the request context
  // with the handle, must not block the calling thread.
  //
  // ~WinHttpRequest() closes the request handle and waits for
  // WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING so that no WinHTTP worker thread can dereference the
  // WinHttpAction after it has been freed. WinHttpAction::StatusCallback() drops every
  // notification that arrives with dwContext == 0, and the context is bound to the handle only by
  // WinHttpSendRequest(). Without the context being bound in the constructor, the destructor waits
  // for a notification that is discarded, on a default-constructed Context that is never
  // cancelled, and the thread is lost for the lifetime of the process.
  //
  // The work runs on a detached thread on purpose: when the bug is present the thread cannot be
  // joined, so the test must still be able to report the failure. On Windows, process exit
  // terminates the leaked thread.
  TEST(WinHttpTransport, RequestThatFailsDuringSetupDoesNotHang)
  {
    auto callCompleted = std::make_shared<std::promise<void>>();
    std::future<void> callCompletedFuture = callCompleted->get_future();

    std::thread([callCompleted]() {
      try
      {
        SendRequestThatFailsDuringSetup();
      }
      catch (...)
      {
        // Expected: the injected Length() failure propagates out of Send(). All this test cares
        // about is that the call returns rather than blocking forever.
      }

      callCompleted->set_value();
    }).detach();

    ASSERT_EQ(std::future_status::ready, callCompletedFuture.wait_for(TransportCallTimeout))
        << "WinHttpTransport::Send did not return within "
        << std::chrono::duration_cast<std::chrono::seconds>(TransportCallTimeout).count()
        << "s. ~WinHttpRequest is blocked waiting for WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING, "
           "which is discarded because the request context was never associated with the request "
           "handle.";
  }

  namespace {

    // Non-contiguous stream that copies from an underlying vector via OnRead (so TryReadContiguous
    // returns false and WinHttpRequest::Upload uses ScopedScratchBuffer).
    class NonContiguousBodyStream final : public Azure::Core::IO::BodyStream {
    private:
      std::vector<uint8_t> const& m_data;
      size_t m_offset = 0;
      size_t m_throwAfterBytes = 0;

      size_t OnRead(uint8_t* buffer, size_t count, Azure::Core::Context const&) override
      {
        if (m_throwAfterBytes > 0 && m_offset >= m_throwAfterBytes)
        {
          throw std::runtime_error("Injected mid-upload OnRead failure");
        }
        size_t const toRead = (std::min)(count, m_data.size() - m_offset);
        if (toRead > 0)
        {
          std::memcpy(buffer, m_data.data() + m_offset, toRead);
          m_offset += toRead;
        }
        return toRead;
      }

    public:
      explicit NonContiguousBodyStream(std::vector<uint8_t> const& data, size_t throwAfterBytes = 0)
          : m_data(data), m_throwAfterBytes(throwAfterBytes)
      {
      }

      int64_t Length() const override { return static_cast<int64_t>(m_data.size()); }
      void Rewind() override { m_offset = 0; }
    };

    class LoopbackHttpServer final {
    private:
      SOCKET m_listenSocket = INVALID_SOCKET;
      uint16_t m_port = 0;

    public:
      LoopbackHttpServer()
      {
        WSADATA wsaData{};
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
          throw std::runtime_error("WSAStartup failed");
        }

        m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_listenSocket == INVALID_SOCKET)
        {
          throw std::runtime_error("socket() failed");
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = 0;

        if (bind(m_listenSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR
            || listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR)
        {
          closesocket(m_listenSocket);
          throw std::runtime_error("bind/listen failed");
        }

        int addrLen = sizeof(addr);
        if (getsockname(m_listenSocket, reinterpret_cast<sockaddr*>(&addr), &addrLen)
            == SOCKET_ERROR)
        {
          closesocket(m_listenSocket);
          throw std::runtime_error("getsockname failed");
        }
        m_port = ntohs(addr.sin_port);
      }

      ~LoopbackHttpServer()
      {
        if (m_listenSocket != INVALID_SOCKET)
        {
          closesocket(m_listenSocket);
        }
        WSACleanup();
      }

      uint16_t GetPort() const { return m_port; }

      std::vector<uint8_t> AcceptAndReadBody(size_t expectedBodySize, bool sendResponse = true)
      {
        SOCKET client = accept(m_listenSocket, nullptr, nullptr);
        if (client == INVALID_SOCKET)
        {
          return {};
        }

        std::string rawRequest;
        size_t headerEndPos = std::string::npos;
        char temp[4096];
        while (headerEndPos == std::string::npos)
        {
          int n = recv(client, temp, sizeof(temp), 0);
          if (n <= 0)
          {
            closesocket(client);
            return {};
          }
          rawRequest.append(temp, static_cast<size_t>(n));
          headerEndPos = rawRequest.find("\r\n\r\n");
        }

        size_t const bodyStart = headerEndPos + 4;
        std::vector<uint8_t> body(rawRequest.begin() + bodyStart, rawRequest.end());
        body.reserve(expectedBodySize);

        while (body.size() < expectedBodySize)
        {
          int n = recv(client, temp, sizeof(temp), 0);
          if (n <= 0)
          {
            break;
          }
          body.insert(body.end(), temp, temp + n);
        }

        if (sendResponse)
        {
          char const response[]
              = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
          send(client, response, static_cast<int>(sizeof(response) - 1), 0);
        }
        closesocket(client);
        return body;
      }
    };

  } // namespace

  TEST(WinHttpTransport, UploadMemoryBodyStreamAndNonContiguousStream)
  {
    LoopbackHttpServer server;
    Azure::Core::Url const url("http://127.0.0.1:" + std::to_string(server.GetPort()) + "/upload");
    Azure::Core::Http::WinHttpTransport transport;

    // 1. 256 KiB MemoryBodyStream (zero-copy path), plus Rewind() and re-send.
    std::vector<uint8_t> memoryPayload(256 * 1024);
    for (size_t i = 0; i < memoryPayload.size(); ++i)
    {
      memoryPayload[i] = static_cast<uint8_t>((i * 31 + 7) & 0xFF);
    }

    Azure::Core::IO::MemoryBodyStream memoryStream(memoryPayload);
    for (int attempt = 0; attempt < 2; ++attempt)
    {
      memoryStream.Rewind();
      auto serverFuture = std::async(
          std::launch::async, [&]() { return server.AcceptAndReadBody(memoryPayload.size()); });

      Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url, &memoryStream);
      auto response = transport.Send(request, Azure::Core::Context{});
      EXPECT_EQ(Azure::Core::Http::HttpStatusCode::Ok, response->GetStatusCode());
      EXPECT_EQ(memoryPayload, serverFuture.get());
    }

    // 2. 2.5 MiB MemoryBodyStream (exceeds 1 MiB MaximumUploadChunkSize to exercise multi-chunk
    // zero-copy loop).
    std::vector<uint8_t> largeMemoryPayload(2 * 1024 * 1024 + 512 * 1024);
    for (size_t i = 0; i < largeMemoryPayload.size(); ++i)
    {
      largeMemoryPayload[i] = static_cast<uint8_t>((i * 13 + 5) & 0xFF);
    }

    Azure::Core::IO::MemoryBodyStream largeMemoryStream(largeMemoryPayload);
    {
      auto serverFuture = std::async(std::launch::async, [&]() {
        return server.AcceptAndReadBody(largeMemoryPayload.size());
      });

      Azure::Core::Http::Request request(
          Azure::Core::Http::HttpMethod::Put, url, &largeMemoryStream);
      auto response = transport.Send(request, Azure::Core::Context{});
      EXPECT_EQ(Azure::Core::Http::HttpStatusCode::Ok, response->GetStatusCode());
      EXPECT_EQ(largeMemoryPayload, serverFuture.get());
    }

    // 3. 150 KiB NonContiguousBodyStream (multiple 64 KiB chunks via ScopedScratchBuffer).
    std::vector<uint8_t> stagedPayload(150 * 1024);
    for (size_t i = 0; i < stagedPayload.size(); ++i)
    {
      stagedPayload[i] = static_cast<uint8_t>((i * 17 + 3) & 0xFF);
    }

    NonContiguousBodyStream stagedStream(stagedPayload);
    auto serverFuture = std::async(
        std::launch::async, [&]() { return server.AcceptAndReadBody(stagedPayload.size()); });

    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url, &stagedStream);
    auto response = transport.Send(request, Azure::Core::Context{});
    EXPECT_EQ(Azure::Core::Http::HttpStatusCode::Ok, response->GetStatusCode());
    EXPECT_EQ(stagedPayload, serverFuture.get());
  }

  TEST(WinHttpTransport, UploadFailureClosesRequestHandleBeforeUnwind)
  {
    LoopbackHttpServer server;
    Azure::Core::Url const url(
        "http://127.0.0.1:" + std::to_string(server.GetPort()) + "/upload-fail");
    Azure::Core::Http::WinHttpTransport transport;

    std::vector<uint8_t> payload(128 * 1024, 0x5A);
    // Throw after the first 64 KiB chunk has been written to WinHTTP.
    NonContiguousBodyStream failingStream(payload, 64 * 1024);

    auto serverFuture = std::async(
        std::launch::async, [&]() { return server.AcceptAndReadBody(64 * 1024, false); });

    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Put, url, &failingStream);
    EXPECT_THROW(
        static_cast<void>(transport.Send(request, Azure::Core::Context{})), std::runtime_error);
    serverFuture.wait();
  }

}}} // namespace Azure::Core::Test

#endif // AZ_PLATFORM_WINDOWS && BUILD_TRANSPORT_WINHTTP_ADAPTER
