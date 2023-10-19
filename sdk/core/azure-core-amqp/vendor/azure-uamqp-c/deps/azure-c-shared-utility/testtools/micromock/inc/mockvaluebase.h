// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MOCKVALUEBASE_H
#define MOCKVALUEBASE_H

#pragma once

#include "stdafx.h"

class CMockValueBase
{
public:
    virtual ~CMockValueBase();

public:
    virtual std::tstring ToString() const = 0;
    virtual bool EqualTo(_In_ const CMockValueBase* right) = 0;
};

#endif // MOCKVALUEBASE_H
