/**
 * @file mystring.h
 * The file contains declaration of classes used to represent string data.
 *
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__MYSTRING_H__
#define __KARBAZOL_DRAGNDROP_2_0__MYSTRING_H__

#include <stddef.h>
#include <utils.h>
#include <growarry.h>

#ifndef EXCEPTION_EXECUTE_HANDLER
#define EXCEPTION_EXECUTE_HANDLER       1
#endif

bool isBadReadPtr(void* p, size_t size);
bool isBadWritePtr(void* p, size_t size);

/**
 * Template class representing traits of string element.
 */
template<class Type>
struct MyStringTraits
{
    typedef Type type;

    /**
     * Length of string.
     *
     * @return Number of elements in zero terminated array.
     */
    static size_t strlen(const type*);

    /**
     * Copies one string to another.
     */
    static type* strcpy(type*, const type*);
    static int strcmp(const type* s1, const type* s2);
    static const type* pathDelim;
    static const type* slash;
    static const type* backSlash;
    static const type* tab;
    static const type* cr;
    static const type* lf;
};

void myStringLock();
void myStringUnlock();
/*
    When the dll is unloaded, Dll::Main calls all finalizers in order reverse to registration order.
    MyString::~MyString requires lockString to be alive.
    If some finalizer destructs something containing MyString,
    that finalizer must be registered after finalizer for lockString;
    if it does not happen implicitly with using lockString before that finalizer is registered,
    it must be done explicitly via initializeStringLock().
*/
void initializeStringLock();

/**
 * Utility class to wrap locking/unlocking of string inter-thread guard.
 */
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

/**
 * Normalizes the path. Converts slashes to backslashes. Removes last backslash.
 */
template<class StringType>
void normalizePath(StringType& path)
{
    StringType::CharType* p = path;
    if (p != 0)
    {
        for(;*p;p++)
        {
            if (*p == *StringType::Traits::slash)
                *p = *StringType::Traits::pathDelim;
        }
        if (*--p == *StringType::Traits::pathDelim)
            *p = 0;
    }
}

template<class StringType>
StringType lstrip(const StringType& s)
{
    const StringType::CharType* p = s;
    if (!p||!StringType::Traits::strlen(p))
    {
        return StringType();
    }

    for (;*p && *p <= 32;++p);

    return StringType(p);
}

template<class StringType>
StringType rstrip(const StringType& s)
{
    StringType res(s);

    size_t i;
    for (i = s.length(); i && s[i-1] <= 32; --i);

    if (i < s.length())
    {
        res.length(i);
    }

    return res;
}

template<class StringType>
StringType strip(const StringType& s)
{
    StringType tmp = rstrip<StringType>(s);
    StringType res = lstrip<StringType>(tmp);
    return res;
}

template<class StringType>
StringType substring(const StringType& s, size_t offset,
        size_t len=static_cast<size_t>(-1))
{
    if (offset >= s.length())
    {
        return StringType();
    }

    const StringType::CharType* p = s + offset;

    StringType res(p);

    res.length(min(s.length() - offset, len));

    return res;
}

template<class StringType>
StringType wrapString(const StringType& s, size_t maxChars, size_t tabLen=8)
{
    tabLen;
    if (!s || StringType::Traits::strlen(s) <= maxChars)
        return s;

    GrowOnlyArray<StringType> lines;
    wrapString(s, maxChars, lines, tabLen);

    size_t resultLength = lines.size() - 1; // Number of LFs
    size_t i;
    for (i = 0; i < lines.size(); ++i)
    {
        resultLength += StringType::Traits::strlen(lines[i]);
    }
    StringType res;
    res.length(resultLength);

    StringType::CharType* p = res;
    for (i = 0; i < lines.size(); ++i)
    {
        size_t len = StringType::Traits::strlen(lines[i]);
        memmove(p, lines[i], len*sizeof(*p));
        p += len;
        *p++ = *StringType::Traits::lf;
    }
    p[-1] = 0;

    return res;
}

template<class StringType>
void wrapString(const StringType& s, size_t maxChars,
        GrowOnlyArray<StringType>& lines, size_t tabLen=8)
{
    tabLen; /** @todo Tabs should be counted too. */
    lines.clear();

    if (maxChars > s.length())
    {
        lines.append(s);
        return;
    }

    size_t begin = 0;
    size_t pos   = 0;
    size_t candidate = 0;
    const StringType::CharType* p = s;

    for (pos = 0; p[pos]; pos++)
    {
        if (pos - begin > maxChars)
        {
            if (candidate > begin)
            {
                lines.append(substring(s, begin, candidate - begin));
                begin = candidate + 1;
            }
        }

        if (p[pos] == 32)
        {
            candidate = pos;
        }
    }

    lines.append(substring(s, begin));
}
/**
 * Main template class to represent string data
 */
template<class Type, class Traits=MyStringTraits<Type> >
class MyString
{
public:
    typedef typename Traits::type CharType;
    typedef typename Traits Traits;
private:
    struct StringData
    {
        size_t mutable volatile refCount;
        // Not including null terminator
        size_t allocated;
        // Not including null terminator
        size_t len;
        CharType data[1];
    };
    StringData* data;
    StringData* newData(size_t len)
    {
        size_t totalLen = sizeof(StringData) + len * sizeof(CharType);

        /*
         * The following case is impossible I guess, but
         * Intel Source Checker thinks it is possible because integer overflow.
         */
        if (totalLen < sizeof(StringData))
        {
            len = 0;
            totalLen = sizeof(StringData);
        }

        StringData* res = reinterpret_cast<StringData*>(::malloc(totalLen));

        if (res)
        {
            res->refCount = 1;
            res->allocated = len;
            res->len = 0;
            res->data[0] = 0;
        }

        return res;
    }
    StringData* allocData(const CharType* s)
    {
        size_t len = s?Traits::strlen(s):0;
        StringData* res;

        if (len)
        {
            res = newData(len);
            if (res)
            {
                Traits::strcpy(res->data, s);
                res->len = len;
            }
        }
        else
        {
            res = 0;
        }

        return res;
    }
    StringData* reallocData(size_t newLen)
    {
        StringData* p = uniqueData();

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
                if (!p)
                {
                    return p;
                }
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
    StringData* uniqueData()
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

                if (data && old->len)
                {
                    ::memmove(data->data, old->data, old->len*sizeof(CharType));
                    data->len = old->len;
                    data->data[data->len] = 0;
                }
            }
        }

        return data;
    }
public:
    ~MyString()
    {
        freeData();
    }
    MyString(): data(0){}
    MyString(const CharType* p): data(0)
    {
        lockString l;
        if (p)
        {
            data = allocData(p);
        }
    }
    MyString(const MyString& s): data(s.data)
    {
        if (&s != this)
        {
            lockString l;

            if (data)
            {
                ASSERT(data->len <= data->allocated);
                data->refCount++;
            }
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
        if (!s)
        {
            freeData();
        }
        else
        {
            size_t size = Traits::strlen(s);
            if (!data)
            {
                data = newData(size);
            }
            else
            {
                lockString l;

                if (data->refCount > 1)
                {
                    freeData();
                    data = newData(size);
                }
                else // data->refCount == 1;
                {
                    if (size > data->allocated)
                    {
                        data = reallocData(size);
                    }
                }
            }
            if (data)
            {
                Traits::strcpy(data->data, s);
                data->len = size;
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
        lockString l;

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
        if (length())
        {
            uniqueData();
            if (data->data[data->len-1] != *Traits::pathDelim)
                operator += (Traits::pathDelim);
        }
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
            data = reallocData(value);
        }

        if (data)
        {
            data->len = value;
            data->data[value] = 0;
        }
    }
};

/**
 * Multibyte string
 */
typedef MyString<char> MyStringA;

/**
 * Unicode string
 */
typedef MyString<wchar_t> MyStringW;

bool splitAndAbsPath(const wchar_t* path, MyStringW& prefix, MyStringW& suffix);

MyStringA w2a(const wchar_t* s, int cp);
MyStringW a2w(const char* s, int cp);

#endif // __KARBAZOL_DRAGNDROP_2_0__MYSTRING_H__
