// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "azure/core/Uri.h"

namespace Azure { namespace Storage {

class StorageUriBuilder
{
public:
    explicit StorageUriBuilder(Azure::Core::Http::Uri uri);

    const std::string& GetScheme() const;
    void SetScheme(std::string scheme);

    const std::string& GetHost() const;
    void SetHost(std::string host);

    const std::string& GetPath const;
    void SetPath(std::string path);

    const std::string& GetQuery() const;
    void SetQuery(std::string query);

    uint16_t GetPort() const;
    void SetPort(uint16_t port);

    const std::string& GetAccountName() const;
    void SetAccountName(std::string accountName);

    const std::string& GetSas() const;
    void SetSas(std::string sas);

private:
    Azure::Core::Http::Uri mUri;
    std::string mAccountName;
    std::string mSas;
};

}}
