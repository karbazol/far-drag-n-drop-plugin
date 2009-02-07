#include "dll.h"
#include "thrdpool.h"
#include "wrkrthrd.h"

void ThreadPool::kill(ThreadPool* p)
{
    delete p;
}

ThreadPool* ThreadPool::instance()
{
    static ThreadPool* p = 0;
    if (!p)
    {
        p = new ThreadPool;
        Dll::instance()->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(&kill), p);
    }

    return p;
}

HRESULT ThreadPool::newThread(IDataObject* obj, const wchar_t* destDir)
{
    WorkerThread* thread = new WorkerThread();

    return thread->execute(obj, destDir);
}

void ThreadPool::shutDown()
{
    // @TODO Implement shutdown. All running threads should finish before the function returns
}

// vim: set et ts=4 ai :

