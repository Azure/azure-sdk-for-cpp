// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/diagnostics/log.hpp"

#include "static_curl_transport.hpp"

#include <memory>

#if defined(_MSC_VER)
// C6101 : Returning uninitialized memory '*Mtu'->libcurl calling WSAGetIPUserMtu from WS2tcpip.h
#pragma warning(push)
#pragma warning(disable : 6101)
#endif

#include <curl/curl.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

using Azure::Core::Context;
using Azure::Core::Http::HttpStatusCode;
using Azure::Core::Http::RawResponse;
using Azure::Core::Http::Request;
using Azure::Core::Http::TransportException;

namespace {
constexpr static const char* FailedToGetNewConnectionTemplate
    = "[static impl] Fail to get a new connection for: ";

constexpr static const int HttpWordLen = 4;

template <typename T>
#if defined(_MSC_VER)
#pragma warning(push)
// C26812: The enum type 'CURLoption' is un-scoped. Prefer 'enum class' over 'enum' (Enum.3)
#pragma warning(disable : 26812)
#endif
inline bool SetStaticLibcurlOption(CURL* handle, CURLoption option, T value, CURLcode* outError)
{
  *outError = curl_easy_setopt(handle, option, value);
  return *outError == CURLE_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

class StaticCurlImpl final : public Azure::Core::IO::BodyStream {
private:
  CURL* m_libcurlHandle;
  struct curl_slist* m_headerHandle = NULL;
  Azure::Core::Http::CurlTransportOptions m_options;
  std::unique_ptr<RawResponse> m_response = nullptr;
  std::vector<uint8_t> m_responseData;
  std::vector<uint8_t> m_sendBuffer;
  std::unique_ptr<Azure::Core::IO::BodyStream> m_responseStream;
  bool m_isTransferEncodingChunked = false;

  // Returns the next token from `*begin` to the next separator parsed as T
  // *begin gets updated to the next token after the separator.
  // use the last param to change the return type to other than int
  template <class T = int>
  static T GetNextToken(
      char const** begin,
      char const* const last,
      char const separator,
      std::function<T(std::string)> mutator
      = [](std::string const& value) { return std::stoi(value); })
  {
    auto start = *begin;
    auto end = std::find(start, last, separator);
    // Move the original ptr to one place after the separator
    *begin = end + 1;
    return mutator(std::string(start, end));
  }

  static std::unique_ptr<RawResponse> CreateHTTPResponse(
      char const* const begin,
      char const* const last)
  {
    // set response code, HTTP version and reason phrase (i.e. HTTP/1.1 200 OK)
    auto start = begin + HttpWordLen + 1; // HTTP = 4, / = 1, moving to 5th place for version
    auto majorVersion = GetNextToken(&start, last, '.');
    auto minorVersion = GetNextToken(&start, last, ' ');
    auto statusCode = GetNextToken(&start, last, ' ');
    auto reasonPhrase = GetNextToken<std::string>(
        &start, last, '\r', [](std::string const& value) { return value; });

    // allocate the instance of response to heap with shared ptr
    // So this memory gets delegated outside CurlTransport as a shared_ptr so memory will be
    // eventually released
    return std::make_unique<RawResponse>(
        static_cast<uint16_t>(majorVersion),
        static_cast<uint16_t>(minorVersion),
        HttpStatusCode(statusCode),
        reasonPhrase);
  }

  static void StaticSetHeader(
      Azure::Core::Http::RawResponse& response,
      char const* const first,
      char const* const last)
  {
    // get name and value from header
    auto start = first;
    auto end = std::find(start, last, ':');

    if ((last - first) == 2 && *start == '\r' && *(start + 1) == '\n')
    {
      // Libcurl gives the end of headers as `\r\n`, we just ignore it
      return;
    }

    if (end == last)
    {
      throw std::invalid_argument("Invalid header. No delimiter ':' found.");
    }

    // Always toLower() headers
    auto headerName = Azure::Core::_internal::StringExtensions::ToLower(std::string(start, end));
    start = end + 1; // start value
    while (start < last && (*start == ' ' || *start == '\t'))
    {
      ++start;
    }

    end = std::find(start, last, '\r');
    auto headerValue = std::string(start, end); // remove \r

    response.SetHeader(headerName, headerValue);
  }

  /***************************  CALL BACKS */

  /**
   * @brief This is the function that curl will use to write response into a user provider span
   * Function receives the size of the response and must return this same number, otherwise it is
   * consider that function failed
   *
   * @param contents response data from Curl response
   * @param size size of the curl response data
   * @param nmemb number of blocks in response
   * @param userp this represent a structure linked to response by us before
   * @return int
   */
  static size_t ReceiveInitialResponse(char* contents, size_t size, size_t nmemb, void* userp)
  {
    size_t const expectedSize = size * nmemb;
    std::unique_ptr<RawResponse>* rawResponse = static_cast<std::unique_ptr<RawResponse>*>(userp);

    // First response
    if (*rawResponse == nullptr)
    {
      // parse header to get init data
      *rawResponse = CreateHTTPResponse(contents, contents + expectedSize);
    }
    else
    {
      StaticSetHeader(*(*rawResponse), contents, contents + expectedSize);
    }

    // This callback needs to return the response size or curl will consider it as it failed
    return expectedSize;
  }

  static size_t ReceiveData(void* contents, size_t size, size_t nmemb, void* userp)
  {
    size_t const expectedSize = size * nmemb;
    std::vector<uint8_t>& rawResponse = *(static_cast<std::vector<uint8_t>*>(userp));
    uint8_t* data = static_cast<uint8_t*>(contents);

    rawResponse.insert(rawResponse.end(), data, data + expectedSize);

    // This callback needs to return the response size or curl will consider it as it failed
    return expectedSize;
  }

  static size_t UploadData(void* dst, size_t size, size_t nmemb, void* userdata)
  {
    // Calculate the size of the *dst buffer
    auto destSize = nmemb * size;
    Azure::Core::IO::BodyStream* uploadStream = static_cast<Azure::Core::IO::BodyStream*>(userdata);

    // Terminate the upload if the destination buffer is too small
    if (destSize < 1)
    {
      throw Azure::Core::Http::TransportException("Not enough size to continue to upload data.");
    }

    // Copy as many bytes as possible from the stream to libcurl's destination buffer
    return uploadStream->Read(static_cast<uint8_t*>(dst), destSize);
  }

  size_t OnRead(uint8_t* buffer, size_t count, Azure::Core::Context const& context) override
  {
    return m_responseStream->Read(buffer, count, context);
  }

public:
  StaticCurlImpl(
      Azure::Core::Http::CurlTransportOptions const& options
      = Azure::Core::Http::CurlTransportOptions())
      : m_options(options)
  {
    // ******************************************************************
    // ***************************************************  INIT ******
    // ******************************************************************
    m_libcurlHandle = curl_easy_init();
    if (!m_libcurlHandle)
    {
      throw Azure::Core::Http::TransportException("Failed to create libcurl handle");
    }
  }

  ~StaticCurlImpl()
  {
    if (m_headerHandle)
    {
      curl_slist_free_all(m_headerHandle);
    }

    if (m_libcurlHandle)
    {
      curl_easy_cleanup(m_libcurlHandle);
    }
  }

  std::unique_ptr<RawResponse> Send(Request& request, Context const& context)
  {
    context.ThrowIfCancelled();
    uint16_t port = request.GetUrl().GetPort();
    std::string const& host = request.GetUrl().GetScheme() + request.GetUrl().GetHost()
        + (port != 0 ? std::to_string(port) : "");

    // ******************************************************************
    // ***************************************************  SET UP ******
    // ******************************************************************
    CURLcode result;
    // Libcurl setup before open connection (url, connect_only, timeout)
    if (!SetStaticLibcurlOption(
            m_libcurlHandle, CURLOPT_URL, request.GetUrl().GetAbsoluteUrl().data(), &result))
    {
      throw Azure::Core::Http::TransportException(
          FailedToGetNewConnectionTemplate + host + ". " + std::string(curl_easy_strerror(result)));
    }
    if (port != 0 && !SetStaticLibcurlOption(m_libcurlHandle, CURLOPT_PORT, port, &result))
    {
      throw Azure::Core::Http::TransportException(
          FailedToGetNewConnectionTemplate + host + ". " + std::string(curl_easy_strerror(result)));
    }
    if (!m_options.Proxy.empty())
    {
      if (!SetStaticLibcurlOption(m_libcurlHandle, CURLOPT_PROXY, m_options.Proxy.c_str(), &result))
      {
        throw Azure::Core::Http::TransportException(
            FailedToGetNewConnectionTemplate + host + ". Failed to set proxy to:" + m_options.Proxy
            + ". " + std::string(curl_easy_strerror(result)));
      }
    }
    if (!m_options.CAInfo.empty())
    {
      if (!SetStaticLibcurlOption(
              m_libcurlHandle, CURLOPT_CAINFO, m_options.CAInfo.c_str(), &result))
      {
        throw Azure::Core::Http::TransportException(
            FailedToGetNewConnectionTemplate + host + ". Failed to set CA cert to:"
            + m_options.CAInfo + ". " + std::string(curl_easy_strerror(result)));
      }
    }
    long sslOption = 0;
    if (!m_options.SslOptions.EnableCertificateRevocationListCheck)
    {
      sslOption |= CURLSSLOPT_NO_REVOKE;
    }
    if (!SetStaticLibcurlOption(m_libcurlHandle, CURLOPT_SSL_OPTIONS, sslOption, &result))
    {
      throw Azure::Core::Http::TransportException(
          FailedToGetNewConnectionTemplate + host + ". Failed to set ssl options to long bitmask:"
          + std::to_string(sslOption) + ". " + std::string(curl_easy_strerror(result)));
    }
    if (!m_options.SslVerifyPeer)
    {
      if (!SetStaticLibcurlOption(m_libcurlHandle, CURLOPT_SSL_VERIFYPEER, 0L, &result))
      {
        throw Azure::Core::Http::TransportException(
            FailedToGetNewConnectionTemplate + host + ". Failed to disable ssl verify peer." + ". "
            + std::string(curl_easy_strerror(result)));
      }
    }

    if (m_options.NoSignal)
    {
      if (!SetStaticLibcurlOption(m_libcurlHandle, CURLOPT_NOSIGNAL, 1L, &result))
      {
        throw Azure::Core::Http::TransportException(
            FailedToGetNewConnectionTemplate + host
            + ". Failed to set NOSIGNAL option for libcurl. "
            + std::string(curl_easy_strerror(result)));
      }
    }

    if (m_options.ConnectionTimeout != Azure::Core::Http::_detail::DefaultConnectionTimeout)
    {
      if (!SetStaticLibcurlOption(
              m_libcurlHandle, CURLOPT_CONNECTTIMEOUT_MS, m_options.ConnectionTimeout, &result))
      {
        throw Azure::Core::Http::TransportException(
            FailedToGetNewConnectionTemplate + host + ". Fail setting connect timeout to: "
            + std::to_string(m_options.ConnectionTimeout.count()) + ". "
            + std::string(curl_easy_strerror(result)));
      }
    }

    // headers sep-up
    auto const& headers = request.GetHeaders();
    if (headers.size() > 0)
    {
      for (auto const& header : headers)
      {
        auto newHandle
            = curl_slist_append(m_headerHandle, (header.first + ":" + header.second).c_str());
        if (newHandle == NULL)
        {
          throw Azure::Core::Http::TransportException("Failing creating header list for libcurl");
        }
        m_headerHandle = newHandle;
      }
      if (!SetStaticLibcurlOption(m_libcurlHandle, CURLOPT_HTTPHEADER, m_headerHandle, &result))
      {
        throw Azure::Core::Http::TransportException(". Failed to set header.");
      }
    }

    // Callback set up
    if (!SetStaticLibcurlOption(
            m_libcurlHandle, CURLOPT_HEADERFUNCTION, ReceiveInitialResponse, &result))
    {
      throw Azure::Core::Http::TransportException(
          FailedToGetNewConnectionTemplate + host + ". Failed to set headers callback." + ". "
          + std::string(curl_easy_strerror(result)));
    }

    if (!SetStaticLibcurlOption(
            m_libcurlHandle, CURLOPT_HEADERDATA, static_cast<void*>(&m_response), &result))
    {
      throw Azure::Core::Http::TransportException(
          FailedToGetNewConnectionTemplate + host + ". Failed to set headers data." + ". "
          + std::string(curl_easy_strerror(result)));
    }
    if (!SetStaticLibcurlOption(m_libcurlHandle, CURLOPT_WRITEFUNCTION, ReceiveData, &result))
    {
      throw Azure::Core::Http::TransportException(
          FailedToGetNewConnectionTemplate + host + ". Failed to set data callback." + ". "
          + std::string(curl_easy_strerror(result)));
    }

    if (!SetStaticLibcurlOption(
            m_libcurlHandle, CURLOPT_WRITEDATA, static_cast<void*>(&m_responseData), &result))
    {
      throw Azure::Core::Http::TransportException(
          FailedToGetNewConnectionTemplate + host + ". Failed to set write data." + ". "
          + std::string(curl_easy_strerror(result)));
    }

    // Method set up
    auto const& method = request.GetMethod();
    if (method == Azure::Core::Http::HttpMethod::Delete)
    {
      if (!SetStaticLibcurlOption(m_libcurlHandle, CURLOPT_CUSTOMREQUEST, "DELETE", &result))
      {
        throw Azure::Core::Http::TransportException(
            FailedToGetNewConnectionTemplate + host + ". Failed to set DELETE Method." + ". "
            + std::string(curl_easy_strerror(result)));
      }
    }
    else if (method == Azure::Core::Http::HttpMethod::Patch)
    {
      if (!SetStaticLibcurlOption(m_libcurlHandle, CURLOPT_CUSTOMREQUEST, "PATCH", &result))
      {
        throw Azure::Core::Http::TransportException(
            FailedToGetNewConnectionTemplate + host + ". Failed to set PATCH Method." + ". "
            + std::string(curl_easy_strerror(result)));
      }
    }
    else if (method == Azure::Core::Http::HttpMethod::Head)
    {
      if (!SetStaticLibcurlOption(m_libcurlHandle, CURLOPT_NOBODY, 1L, &result))
      {
        throw Azure::Core::Http::TransportException(
            FailedToGetNewConnectionTemplate + host + ". Failed to set HEAD Method." + ". "
            + std::string(curl_easy_strerror(result)));
      }
    }
    else if (method == Azure::Core::Http::HttpMethod::Post)
    {
      // Adds special header "Expect:" for libcurl to avoid sending only headers to server and wait
      // for a 100 Continue response before sending a PUT method
      auto newHandle = curl_slist_append(m_headerHandle, "Expect:");
      if (newHandle == NULL)
      {
        throw Azure::Core::Http::TransportException("Failing adding Expect header for POST");
      }
      m_headerHandle = newHandle;

      m_sendBuffer = request.GetBodyStream()->ReadToEnd();
      m_sendBuffer.emplace_back('\0'); // the body is expected to be null terminated
      if (!SetStaticLibcurlOption(
              m_libcurlHandle,
              CURLOPT_POSTFIELDS,
              reinterpret_cast<char*>(m_sendBuffer.data()),
              &result))
      {
        throw Azure::Core::Http::TransportException(
            FailedToGetNewConnectionTemplate + host + ". Failed to set POST Data." + ". "
            + std::string(curl_easy_strerror(result)));
      }
    }
    else if (method == Azure::Core::Http::HttpMethod::Put)
    {
      // As of CURL 7.12.1 CURLOPT_PUT is deprecated.  PUT requests should be made using
      // CURLOPT_UPLOAD

      // Adds special header "Expect:" for libcurl to avoid sending only headers to server and wait
      // for a 100 Continue response before sending a PUT method
      auto newHandle = curl_slist_append(m_headerHandle, "Expect:");
      if (newHandle == NULL)
      {
        throw Azure::Core::Http::TransportException("Failing adding Expect header for POST");
      }
      m_headerHandle = newHandle;

      if (!SetStaticLibcurlOption(m_libcurlHandle, CURLOPT_UPLOAD, 1L, &result))
      {
        throw Azure::Core::Http::TransportException(
            FailedToGetNewConnectionTemplate + host + ". Failed to set Curl handle to PUT mode"
            + ". " + std::string(curl_easy_strerror(result)));
      }

      if (!SetStaticLibcurlOption(m_libcurlHandle, CURLOPT_READFUNCTION, UploadData, &result))
      {
        throw Azure::Core::Http::TransportException(
            FailedToGetNewConnectionTemplate + host + ". Failed to set Upload callback" + ". "
            + std::string(curl_easy_strerror(result)));
      }

      auto uploadStream = request.GetBodyStream();
      if (!SetStaticLibcurlOption(
              m_libcurlHandle, CURLOPT_READDATA, static_cast<void*>(uploadStream), &result))
      {
        throw Azure::Core::Http::TransportException(
            FailedToGetNewConnectionTemplate + host + ". Failed to set Upload body Stream" + ". "
            + std::string(curl_easy_strerror(result)));
      }
      if (!SetStaticLibcurlOption(
              m_libcurlHandle,
              CURLOPT_INFILESIZE,
              static_cast<curl_off_t>(uploadStream->Length()),
              &result))
      {
        throw Azure::Core::Http::TransportException(
            FailedToGetNewConnectionTemplate + host + ". Failed to set Upload body Stream Size"
            + ". " + std::string(curl_easy_strerror(result)));
      }
    }

    // ******************************************************************
    // ***************************************************  PERFORM & RECEIVE ******
    // ******************************************************************
    // curl_easy_perform will block the program until all the response is received
    auto performResult = curl_easy_perform(m_libcurlHandle);
    if (performResult != CURLE_OK)
    {
      throw Azure::Core::Http::TransportException(
          FailedToGetNewConnectionTemplate + host + ". "
          + std::string(curl_easy_strerror(performResult)));
    }

    // At this point, libcurl has read all the response from server successfully and the response
    // was written to `m_responseData`. Let's init a memoryBodyStream to enable this class to bahave
    // as a bodyStream.
    m_responseStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(m_responseData);

    auto const& responseHeaders = m_response->GetHeaders();
    auto isTransferEncodingHeaderInResponse = responseHeaders.find("transfer-encoding");
    if (isTransferEncodingHeaderInResponse != responseHeaders.end())
    {
      // if `chunked` is found inside the transfer-encoding, we activate the
      // isTransferEncodingChunked flag. The static-libcurl implementation handles downloading a
      // chunked response. The entire response is already downloaded, so we will just use the header
      // to modify the way the body stream behaves so it acts as an unknown-size memory stream.
      auto headerValue = isTransferEncodingHeaderInResponse->second;
      m_isTransferEncodingChunked = headerValue.find("chunked") != std::string::npos;
    }

    // ******************************************************************
    // ***************************************************  Return response ******
    // ******************************************************************
    return std::move(m_response);
  }

  int64_t Length() const override
  {
    return m_isTransferEncodingChunked ? -1 : m_responseStream->Length();
  }

  void Rewind() override { return m_responseStream->Rewind(); }
};
} // namespace

std::unique_ptr<RawResponse> Azure::Core::Http::StaticCurlTransport::Send(
    Request& request,
    Context const& context)
{
  auto client = std::make_unique<StaticCurlImpl>(m_options);
  auto response = client->Send(request, context);
  response->SetBodyStream(std::move(client));
  return response;
}
