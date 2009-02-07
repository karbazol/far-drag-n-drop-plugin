// $Id$

#ifndef __KARBAZOL_DRAGNDROP_2_0__MYSTRING_H__
#define __KARBAZOL_DRAGNDROP_2_0__MYSTRING_H__

#include <stddef.h>
#include "utils.h"

#define MYSTRING_COOKIE 0x4b43534b

struct CharTraits
{
    typedef char type;
    static size_t strlen(const type*);
    static type* strcpy(type*, const type*);
    static int strcmp(const type* s1, const type* s2);
    static type* pathDelim;
};

struct WideCharTraits
{
    typedef wchar_t type;
    static size_t strlen(const type*);
    static type* strcpy(type*, const type*);
    static int strcmp(const type* s1, const type* s2);
    static type* pathDelim;
};

void myStringLock();
void myStringUnlock();

class lockString
{
public:
    lockString()
    {
        myStringLock();
    }
    ~lockString()
    {
        myStringUnlock();
    }
};

template<class Traits>
class MyString
{
public:
    typedef typename Traits::type CharType;
private:
    struct StringData
    {
        int cookie;
        size_t refCount;
        // Not including null terminator
        size_t allocated;
        // Not including null terminator
        size_t len;
        CharType data[1];
    };
    StringData* data;
    StringData* tryData(const CharType* s)
    {
        if (!s)
            return 0;
        __try
        {
            StringData* res = reinterpret_cast<StringData*>((char*)s - offsetof(StringData,data));
            if (res->cookie == MYSTRING_COOKIE)
                return res;
            else
                return 0;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return 0;
        }
    }
    StringData* newData(size_t len)
    {
        size_t totalLen = sizeof(StringData) + len * sizeof(CharType);

        StringData* res = reinterpret_cast<StringData*>(::malloc(totalLen));

        res->cookie = MYSTRING_COOKIE;
        res->refCount = 1;
        res->allocated = len;
        res->len = 0;
        res->data[0] = 0;

        return res;
    }
    StringData* allocData(const CharType* s)
    {
        size_t len = s?Traits::strlen(s):0;
        StringData* res;

        if (len)
        {
            res = newData(len);
            Traits::strcpy(res->data, s);
            res->len = len;
        }
        else
        {
            res = 0;
        }

        return res;
    }
    StringData* reallocData(StringData* p, size_t newLen)
    {
        uniqueData();

        if (!p)
        {
            p = newData(newLen);
        }
        else
        {
            if (newLen > p->allocated)
            {
                size_t totalLen = sizeof(StringData) + newLen * sizeof(CharType);
                p = reinterpret_cast<StringData*>(::realloc(p, totalLen));
                p->allocated = newLen;
            }

            p->len = newLen;
            p->data[newLen] = 0;
        }

        return p;
    }
    void freeData()
    {
        lockString l;

        if (data)
        {
            ASSERT(data->len <= data->allocated);

            data->refCount--;
            if (!data->refCount)
                ::free(data);
            data = 0;
        }
    }
    void uniqueData()
    {
        lockString l;

        if (data)
        {
            ASSERT(data->len <= data->allocated);
            if (data->refCount > 1)
            {
                StringData* old = data;
                old->refCount--;
                data = newData(data->allocated);

                if (old->len)
                {
                    ::memmove(data->data, old->data, old->len*sizeof(CharType));
                    data->len = old->len;
                    data->data[data->len] = 0;
                }
            }
        }
    }
public:
    ~MyString()
    {
        freeData();
    }
    MyString(): data(0){}
    MyString(const CharType* p): data(0)
    {
        if (p != data->data)
        {
            lockString l;
            data = tryData(p);
            if (data)
                data->refCount++;
            else
                data = allocData(p);
        }
    }
    MyString(const MyString& s): data(s.data)
    {
        if (&s != this)
        {
            lockString l;

            if (data)
                ASSERT(data->len <= data->allocated);
                data->refCount++;
        }
    }
    MyString& operator=(const MyString& m)
    {
        if (&m != this)
        {
            lockString l;

            freeData();

            data = m.data;
            if (data)
            {
                ASSERT(data->len <= data->allocated);
                data->refCount++;
            }
        }

        return *this;
    }
    MyString& operator=(const CharType* s)
    {
        if (data->data != s)
        {
            freeData();

            if (s)
            {
                data = tryData(s);
                if (data)
                    data->refCount++;
                else
                    data = allocData(s);
            }
        }

        return *this;
    }
    bool operator ==(const CharType* s) const
    {
        if (s == 0)
        {
            return (data == 0) || (data->len == 0);
        }
        else if (data == 0)
        {
            return (0 == Traits::strlen(s));
        }
        else if (s == data->data)
        {
            return true;
        }
        else 
        {
            return (0 == Traits::strcmp(s, data->data));
        }
    }
    bool operator ==(const MyString& s) const
    {
        if (this == &s)
        {
            return true;
        }
        else
        {
            return operator ==(static_cast<const CharType*>(s));
        }
    }
    bool operator !() const
    {
        return (data == 0) || (data->len == 0);
    }
    operator const CharType*() const
    {
        if (!data)
            return 0;
        return data->data;
    }
    operator CharType*()
    {
        if (!data)
            return 0;

        uniqueData();

        return data->data;
    }
    MyString& operator +=(const CharType* s)
    {
        if (!data)
        {
            data = allocData(s);
        }
        else
        {
            uniqueData();
            ASSERT(data->len <= data->allocated);

            size_t size = s?Traits::strlen(s):0;

            if (size)
            {
                size_t old_len = data->len;

                length(old_len + size);

                Traits::strcpy(data->data + old_len, s);
            }
        }

        return *this;
    }
    MyString& operator += (const MyString& s)
    {
        return operator += (static_cast<const CharType*>(s));
    }
    MyString operator + (const CharType* s) const
    {
        MyString res(*this);

        return res += s;
    }
    MyString operator + (const MyString& s) const
    {
        return operator + (static_cast<const CharType*>(s));
    }
    MyString& operator /= (const CharType* s)
    {
        operator += (Traits::pathDelim);
        return operator += (s);
    }
    MyString& operator /= (const MyString& s)
    {
        return operator /= (static_cast<const CharType*>(s));
    }
    MyString operator / (const CharType* s) const
    {
        MyString res(*this);

        return res /= s;
    }
    MyString operator / (const MyString& s) const
    {
        return operator / (static_cast<const CharType*>(s));
    }

    size_t length() const
    {
        return data?data->len:0;
    }

    void length(size_t value)
    {
        uniqueData();

        if (!data)
        {
            data = newData(value);
        }
        else if (value > data->allocated)
        {
            data = reallocData(data, value);
        }

        data->len = value;
        data->data[value] = 0;
    }
};

typedef MyString<CharTraits> MyStringA;
typedef MyString<WideCharTraits> MyStringW;

bool splitAndAbsPath(const wchar_t* path, MyStringW& prefix, MyStringW& suffix);

MyStringA w2a(const wchar_t* s, int cp);
MyStringW a2w(const char* s, int cp);

#endif // __KARBAZOL_DRAGNDROP_2_0__MYSTRING_H__
