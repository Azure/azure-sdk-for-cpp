// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_WINDOWS) && defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/win_http_transport.hpp"
#include "azure/core/io/body_stream.hpp"
#include "azure/core/url.hpp"

#include <chrono>
#include <cstdint>
#include <future>
#include <memory>
#include <stdexcept>
#include <thread>

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

}}} // namespace Azure::Core::Test

#endif // AZ_PLATFORM_WINDOWS && BUILD_TRANSPORT_WINHTTP_ADAPTER
