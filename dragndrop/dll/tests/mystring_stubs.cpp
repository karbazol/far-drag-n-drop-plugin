#include <list>
#include <dll/dll.h>

struct CallBackInfo_
{
    PdllCallBack callBack;
    void* data;
};

static std::list<CallBackInfo_> callbacks;

Dll::~Dll()
{
    std::list<CallBackInfo_>::const_iterator p;

    for (p = callbacks.begin(); p != callbacks.end(); p++)
    {
        p->callBack(p->data);
    }
}

void Dll::registerProcessEndCallBack(PdllCallBack func, void* data)
{
    CallBackInfo_ p = {func, data};

    callbacks.push_back(p);
}

Dll* Dll::instance()
{
    static Dll dll;

    return &dll;
}

// vim: set et ts=4 sw=4 ai :

