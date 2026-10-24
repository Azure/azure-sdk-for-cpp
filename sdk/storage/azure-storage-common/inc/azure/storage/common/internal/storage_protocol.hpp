// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/exception.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include <utility>

namespace Azure { namespace Storage { namespace _internal {

  template <class Operation>
  auto InvokeStorageProtocol(Operation&& operation)
      -> decltype(std::forward<Operation>(operation)())
  {
    try
    {
      return std::forward<Operation>(operation)();
    }
    catch (StorageException const&)
    {
      throw;
    }
    catch (Core::RequestFailedException& error)
    {
      if (!error.RawResponse)
      {
        throw;
      }
      throw StorageException::CreateFromResponse(std::move(error.RawResponse));
    }
  }

}}} // namespace Azure::Storage::_internal
