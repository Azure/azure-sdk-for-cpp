// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <mutex>
#include <string>

namespace Azure { namespace Storage {

class StorageSharedKeyCredential
{
public:
    StorageSharedKeyCredential(std::string accountName, std::string accountKey);

    const std::string& GetAccountName() const;
    const std::string& GetAccountKey() const;
    void SetAccountKey(std::string accountKey);

private:
    std::mutex mAccountKeyMutex;
    std::string mAccountName;
    std::string mAccountKey;
};

}}
