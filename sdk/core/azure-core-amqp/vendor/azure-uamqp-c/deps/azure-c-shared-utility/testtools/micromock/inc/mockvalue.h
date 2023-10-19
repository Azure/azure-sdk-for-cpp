// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MOCKVALUE_H
#define MOCKVALUE_H

#pragma once

#include "stdafx.h"
#include "mockvaluebase.h"

#include <sstream>


template<typename T>
class CMockValue :
    public CMockValueBase
{
public:
    CMockValue(_In_ const T& value) : m_Value(value), m_OriginalValue(m_Value)
    {
    }

    virtual ~CMockValue()
    {
    }

    virtual std::tstring ToString() const
    {
        std::tostringstream strStream;
        strStream << m_Value;
        return strStream.str();
    }

    virtual bool EqualTo(_In_ const CMockValueBase* right)
    {
        return (*this == *(reinterpret_cast<const CMockValue<T>*>(right)));
    }

    void SetValue(_In_ const T& value)
    {
        m_Value = value;
        m_OriginalValue = value;
    }

    const T& GetValue() const
    {
        return m_Value;
    }

protected:
    T m_Value;
    T m_OriginalValue;
};

template<>
class CMockValue <unsigned char>:
    public CMockValueBase
{
public:
    CMockValue(_In_ const unsigned char& value) : m_Value(value), m_OriginalValue(value)
    {
    }

    virtual ~CMockValue()
    {
    }

    virtual std::tstring ToString() const
    {
            std::tostringstream strStream;
            strStream << (unsigned int)m_Value;
            return strStream.str();
    }

    virtual bool EqualTo(_In_ const CMockValueBase* right)
    {
            return (this->m_Value == (reinterpret_cast<const CMockValue<unsigned char>*>(right))->m_Value);
    }

    void SetValue(_In_ const unsigned char& value)
    {
        m_Value = value;
        m_OriginalValue = value;
    }

    const unsigned char& GetValue() const
    {
        return m_Value;
    }

protected:
    unsigned char m_Value;
    unsigned char m_OriginalValue;
};

template<>
class CMockValue <void> :
    public CMockValueBase
{
public:
    CMockValue(void)
    {
    }

    virtual ~CMockValue()
    {
    }

    virtual std::tstring ToString() const
    {
        std::tostringstream strStream;
        strStream << "void";
        return strStream.str();
    }

    virtual bool EqualTo(_In_ const CMockValueBase* right)
    {
        (void)right;
        return true;
    }

    void SetValue(void)
    {

    }

    void GetValue() const
    {
        return;
    }

protected:
    unsigned char m_Value;
    unsigned char m_OriginalValue;
};

template<>
class CMockValue <wchar_t *> :
    public CMockValueBase
{
public:
    CMockValue(_In_ const wchar_t * value)
    {
        if (value == NULL)
        {
            m_Value = L"NULL";
            m_OriginalValue = L"NULL";
        }
        else
        {
            m_Value = value;
            m_OriginalValue = value;
        }
    }

    virtual ~CMockValue()
    {
    }

    virtual std::tstring ToString() const
    {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244)
#endif // _MSC_VER
            std::string temp(m_Value.begin(), m_Value.end());

            std::tostringstream strStream;
            strStream << temp.c_str();
            return strStream.str();
#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER
    }

    virtual bool EqualTo(_In_ const CMockValueBase* right)
    {
            return (this->m_Value == (reinterpret_cast<const CMockValue<wchar_t *>*>(right))->m_Value);
    }

    void SetValue(_In_ const wchar_t* value)
    {
        if (value == NULL)
        {
            m_Value = L"NULL";
            m_OriginalValue = L"NULL";
        }
        else
        {
            m_Value = value;
            m_OriginalValue = value;
        }
    }

    const wchar_t* GetValue() const
    {
        return m_Value.c_str();
    }

protected:
    std::wstring m_Value;
    std::wstring m_OriginalValue;
};

template<>
class CMockValue <const wchar_t *> :
    public CMockValueBase
{
public:
    CMockValue(_In_ const wchar_t * value)
    {
        if (value == NULL)
        {
            m_Value = L"NULL";
            m_OriginalValue = L"NULL";
        }
        else
        {
            m_Value = value;
            m_OriginalValue = value;
        }
    }

    virtual ~CMockValue()
    {
    }

    virtual std::tstring ToString() const
    {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244)
#endif // _MSC_VER
            std::string temp(m_Value.begin(), m_Value.end());

            std::tostringstream strStream;
            strStream << temp.c_str();
            return strStream.str();
#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER
    }

    virtual bool EqualTo(_In_ const CMockValueBase* right)
    {
            return (this->m_Value == (reinterpret_cast<const CMockValue<const wchar_t *>*>(right))->m_Value);
    }

    void SetValue(_In_ const wchar_t* value)
    {
        if (value == NULL)
        {
            m_Value = L"NULL";
            m_OriginalValue = L"NULL";
        }
        else
        {
            m_Value = value;
            m_OriginalValue = value;
        }
    }

    const wchar_t* GetValue() const
    {
        return m_Value.c_str();
    }

protected:
    std::wstring m_Value;
    std::wstring m_OriginalValue;
};

template<typename T>
class CMockValue <T*> :
    public CMockValueBase
{
public:
    CMockValue(_In_ T* value)
    {
        m_Value = value;
        m_OriginalValue = value;
    }

    virtual ~CMockValue()
    {
    }

    virtual std::tstring ToString() const
    {
        std::tostringstream strStream;
        if (NULL == m_Value)
        {
            strStream << _T("NULL");
        }
        else
        {
            strStream << m_Value;
        }
        return strStream.str();
    }

    virtual bool EqualTo(_In_ const CMockValueBase* right)
    {
        return (*this == *(reinterpret_cast<const CMockValue<T*>*>(right)));
    }

    void SetValue(_In_ T* value)
    {
        m_Value = value;
        m_OriginalValue = value;
    }

    T* GetValue() const
    {
        return m_Value;
    }

protected:
    T* m_Value;
    T* m_OriginalValue;
};

template<typename T, size_t N>
class CMockValue <T[N]> :
public CMockValueBase
{
public:
    typedef struct arraySave_TAG
    {
        bool isNULL;
        T* copy;
    }arraySave;

    typedef T T_Array_N[N];
    CMockValue(_In_ T* value)
    {
        m_OriginalValue = value;
        if (value == NULL)
        {
            m_Value.isNULL = true;
        }
        else
        {
            m_Value.isNULL = false;
            m_Value.copy = new T[N]();
            for (size_t i = 0; i < N;i++)
            {
                m_Value.copy[i] = value[i];
            }
        }
    }

    virtual ~CMockValue()
    {
        if (m_Value.isNULL ==false)
        {
            delete[] m_Value.copy;
        }
    }

    virtual std::tstring ToString() const
    {
        std::tostringstream strStream;
        if (m_Value.isNULL)
        {
            strStream << "NULL";
        }
        else
        {
            strStream << "{";
            for (size_t i = 0; i < N; i++)
            {
                if (i > 0)
                {
                    strStream << ",";
                }

                strStream << m_Value.copy[i];
            }
            strStream << "}";
        }
        return strStream.str();
    }

    virtual bool EqualTo(_In_ const CMockValueBase* right)
    {
        /*this will be interesting*/
        return (*this == *(reinterpret_cast<const CMockValue<T_Array_N>*>(right)));
    }

    void SetValue(_In_ T* value)
    {
        m_OriginalValue = value;
        if (m_Value.isNULL == true)
        {
            if (value == NULL)
            {
                /*do nothing*/
            }
            else
            {
                m_Value.isNULL = false;
                m_Value.copy = new T[N];
                for (size_t i = 0; i < N; i++)
                {
                    m_Value.copy[i] = value[i];
                }
            }
        }
        else
        {
            if (value == NULL)
            {
                m_Value.isNULL = true;
                delete[]m_Value.copy;
            }
            else
            {
                if (value == m_Value.copy)
                {
                    /*do nothing, we are copying from us to us*/
                }
                else
                {
                    m_Value.isNULL = false;
                    delete[] m_Value.copy;
                    m_Value.copy = new T[N];
                    for (size_t i = 0; i < N; i++)
                    {
                        m_Value.copy[i] = value[i];
                    }
                }
            }
        }
    }

    const T* GetValue() const
    {
        if (m_Value.isNULL == true)
        {
            return NULL;
        }
        else
        {
            return m_Value.copy;
        }
    }

protected:
    arraySave m_Value;
    T* m_OriginalValue;
};

template<typename T, size_t N>
class CMockValue <const T[N]> :
    public CMockValueBase
{
public:
    typedef struct arraySave_TAG
    {
        bool isNULL;
        T* copy;
    }arraySave;

    typedef T T_Array_N[N];
    CMockValue(_In_ const T* value)
    {
        m_OriginalValue = value;
        if (value == NULL)
        {
            m_Value.isNULL = true;
        }
        else
        {
            m_Value.isNULL = false;
            m_Value.copy = new T[N]();
            for (size_t i = 0; i < N; i++)
            {
                m_Value.copy[i] = value[i];
            }
        }
    }

    virtual ~CMockValue()
    {
        if (m_Value.isNULL == false)
        {
            delete[] m_Value.copy;
        }
    }

    virtual std::tstring ToString() const
    {
            std::tostringstream strStream;
            if (m_Value.isNULL)
            {
                strStream << "NULL";
            }
            else
            {
                strStream << "{";
                for (size_t i = 0; i < N; i++)
                {
                    if (i > 0)
                    {
                        strStream << ",";
                    }

                    strStream << m_Value.copy[i];
                }
                strStream << "}";
            }
            return strStream.str();
    }

    virtual bool EqualTo(_In_ const CMockValueBase* right)
    {
            /*this will be interesting*/
            return (*this == *(reinterpret_cast<const CMockValue<const T_Array_N>*>(right)));
    }

    void SetValue(_In_ const T* value)
    {
        m_OriginalValue = value;
        if (m_Value.isNULL == true)
        {
            if (value == NULL)
            {
                /*do nothing*/
            }
            else
            {
                m_Value.isNULL = false;
                m_Value.copy = new T[N];
                for (size_t i = 0; i < N; i++)
                {
                    m_Value.copy[i] = value[i];
                }
            }
        }
        else
        {
            if (value == NULL)
            {
                m_Value.isNULL = true;
                delete[]m_Value.copy;
            }
            else
            {
                if (value == m_Value.copy)
                {
                    /*do nothing, we are copying from us to us*/
                }
                else
                {
                    m_Value.isNULL = false;
                    delete[] m_Value.copy;
                    m_Value.copy = new T[N];
                    for (size_t i = 0; i < N; i++)
                    {
                        m_Value.copy[i] = value[i];
                    }
                }
            }
        }
    }

    const T* GetValue() const
    {
        if (m_Value.isNULL == true)
        {
            return NULL;
        }
        else
        {
            return m_Value.copy;
        }
    }

protected:
    arraySave m_Value;
    const T* m_OriginalValue;
};


template<typename T>
static bool operator==(_In_ const CMockValue<T>& lhs, _In_ const CMockValue<T>& rhs)
{
    return lhs.GetValue() == rhs.GetValue();
}

template<typename T, size_t N>
static bool operator==(_In_ const CMockValue<T[N]>& lhs, _In_ const CMockValue<T[N]>& rhs)
{
    /*comparing two arrays needs to be handled differently than the build in == operator*/
    return(strcmp(lhs.ToString().c_str(), rhs.ToString().c_str()) == 0);
}

#endif // MOCKVALUE_H
