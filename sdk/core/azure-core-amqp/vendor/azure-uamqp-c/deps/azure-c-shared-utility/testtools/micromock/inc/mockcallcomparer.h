// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MOCKCALLCOMPARER_H
#define MOCKCALLCOMPARER_H

#pragma once

#include "stdafx.h"
#include "mockmethodcallbase.h"

class CMockCallComparer
{
public:
    CMockCallComparer() :
        m_IgnoreUnexpectedCalls(false)
    {
    }

    void SetIgnoreUnexpectedCalls(_In_ bool ignoreUnexpectedCalls) { m_IgnoreUnexpectedCalls = ignoreUnexpectedCalls; }
    bool GetIgnoreUnexpectedCalls(void) { return m_IgnoreUnexpectedCalls; }

    virtual bool IsUnexpectedCall(_In_ const CMockMethodCallBase* actualCall) = 0;
    virtual bool IsMissingCall(_In_ const CMockMethodCallBase* actualCall) = 0;
    virtual CMockMethodCallBase* MatchCall(std::vector<CMockMethodCallBase*>& expectedCalls,
        CMockMethodCallBase* actualCall) = 0;

protected:
    bool m_IgnoreUnexpectedCalls;
};

#endif // MOCKCALLCOMPARER_H
