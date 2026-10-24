// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/storage/common/internal/storage_protocol.hpp>

#include <memory>
#include <stdexcept>
#include <type_traits>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test {
  namespace Http = Core::Http;

  TEST(StorageProtocol, PreservesReturnTypesAndForwarding)
  {
    bool invoked = false;
    _internal::InvokeStorageProtocol([&] { invoked = true; });
    EXPECT_TRUE(invoked);
    EXPECT_EQ(_internal::InvokeStorageProtocol([] { return 42; }), 42);
    auto value = _internal::InvokeStorageProtocol([] { return std::make_unique<int>(42); });
    EXPECT_EQ(*value, 42);

    int number = 7;
    auto reference = [&]() -> int& { return number; };
    static_assert(
        std::is_same<decltype(_internal::InvokeStorageProtocol(reference)), int&>::value,
        "Preserve reference return types");
    EXPECT_EQ(&_internal::InvokeStorageProtocol(reference), &number);
    auto rvalueReference = [&]() -> int&& { return std::move(number); };
    static_assert(
        std::is_same<decltype(_internal::InvokeStorageProtocol(rvalueReference)), int&&>::value,
        "Preserve rvalue reference return types");

    struct MoveOnlyOperation
    {
      std::unique_ptr<int> Value;
      explicit MoveOnlyOperation(int value) : Value(std::make_unique<int>(value)) {}
      int operator()() && { return *Value; }
      int operator()() & = delete;
    };
    EXPECT_EQ(_internal::InvokeStorageProtocol(MoveOnlyOperation(9)), 9);
  }

  TEST(StorageProtocol, PreservesExistingExceptions)
  {
    StorageException const* existing = nullptr;
    try
    {
      _internal::InvokeStorageProtocol([&] {
        try
        {
          StorageException error("existing Storage exception");
          error.ErrorCode = "Existing";
          error.AdditionalInformation["Detail"] = "retained";
          error.RawResponse = std::make_unique<Http::RawResponse>(
              1, 1, Http::HttpStatusCode::Conflict, "Conflict");
          throw error;
        }
        catch (StorageException const& error)
        {
          existing = &error;
          throw;
        }
      });
      FAIL() << "Expected StorageException";
    }
    catch (StorageException const& error)
    {
      EXPECT_EQ(&error, existing);
      EXPECT_STREQ(error.what(), "existing Storage exception");
      EXPECT_EQ(error.ErrorCode, "Existing");
      EXPECT_EQ(error.AdditionalInformation.at("Detail"), "retained");
      ASSERT_NE(error.RawResponse, nullptr);
    }
    Core::RequestFailedException const* withoutResponse = nullptr;
    try
    {
      _internal::InvokeStorageProtocol([&] {
        try
        {
          throw Core::RequestFailedException("response parsing failed");
        }
        catch (Core::RequestFailedException const& error)
        {
          withoutResponse = &error;
          throw;
        }
      });
      FAIL() << "Expected Core::RequestFailedException";
    }
    catch (StorageException const&)
    {
      FAIL() << "An exception without an HTTP response must not be translated";
    }
    catch (Core::RequestFailedException const& error)
    {
      EXPECT_EQ(&error, withoutResponse);
      EXPECT_STREQ(error.what(), "response parsing failed");
      EXPECT_EQ(error.RawResponse, nullptr);
    }
    EXPECT_THROW(
        _internal::InvokeStorageProtocol([] { throw std::logic_error("unrelated"); }),
        std::logic_error);
  }

  TEST(StorageProtocol, ConvertsResponseAndTransfersOwnership)
  {
    auto response = std::make_unique<Http::RawResponse>(
        1, 1, Http::HttpStatusCode::BadRequest, "Bad Request");
    response->SetHeader("content-type", "application/xml");
    response->SetHeader("x-ms-request-id", "request-id");
    response->SetHeader("x-ms-client-request-id", "client-id");
    std::string const body = "<Error><Code>InvalidMetadata</Code><Message>Invalid value</Message>"
                             "<Detail>retained</Detail></Error>";
    response->SetBody(std::vector<uint8_t>(body.begin(), body.end()));
    Http::RawResponse const* originalResponse = nullptr;
    try
    {
      _internal::InvokeStorageProtocol([&] {
        try
        {
          Core::RequestFailedException error("generated failure");
          error.RawResponse = std::move(response);
          throw error;
        }
        catch (Core::RequestFailedException const& error)
        {
          originalResponse = error.RawResponse.get();
          throw;
        }
      });
      FAIL() << "Expected StorageException";
    }
    catch (StorageException const& error)
    {
      EXPECT_EQ(error.RawResponse.get(), originalResponse);
      EXPECT_EQ(error.StatusCode, Http::HttpStatusCode::BadRequest);
      EXPECT_EQ(error.ErrorCode, "InvalidMetadata");
      EXPECT_EQ(error.Message, "Invalid value");
      EXPECT_EQ(error.RequestId, "request-id");
      EXPECT_EQ(error.ClientRequestId, "client-id");
      EXPECT_EQ(error.AdditionalInformation.at("Detail"), "retained");
      ASSERT_NE(error.RawResponse, nullptr);
      EXPECT_EQ(error.RawResponse->GetHeaders().at("content-type"), "application/xml");
    }
    EXPECT_EQ(response, nullptr);
  }
}}} // namespace Azure::Storage::Test
