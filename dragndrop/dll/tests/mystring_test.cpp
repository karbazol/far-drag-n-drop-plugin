#include <tut/tut.hpp>
#include <dll/mystring.h>

namespace tut
{
    struct MyStringAData
    {
    };

    typedef test_group<MyStringAData> MyStringAGroup;
    typedef MyStringAGroup::object MyStringAObject;

    MyStringAGroup test_group("MyStringA");

    template<>
    template<>
    void MyStringAObject::test<1>()
    {
        MyStringA a;

        ensure(a == "");
    }

    template<>
    template<>
    void MyStringAObject::test<2>()
    {
        MyStringA a("C://work/");
        MyStringA b(a);

        ensure(b == a);
    }

    template<>
    template<>
    void MyStringAObject::test<3>()
    {
        MyStringA a("C:/work/");

        normalizePath(a);

        ensure(a == "C:\\work");
    }

    template<>
    template<>
    void MyStringAObject::test<4>()
    {
        MyStringA a("    a");

        ensure(lstrip(a) == "a");
    }

    template<>
    template<>
    void MyStringAObject::test<5>()
    {
        MyStringA a("12345678 12345678  12345678");
        ensure(wrapString(a, 8) == "12345678\n12345678\n 12345678");
    }
}

// vim: set et ts=4 sw=4 ai :

