#include "dll.h"
#include "thrdpool.h"
#include "wrkrthrd.h"

class ThreadPoolImpl: public ThreadPool
{
public:
    static void kill(ThreadPoolImpl* p)
    {
        delete p;
    }

    HRESULT newThread(IDataObject* obj, const wchar_t* destDir)
    {
        WorkerThread* thread = new WorkerThread();

        if (!thread)
        {
            return E_OUTOFMEMORY;
        }

        return thread->execute(obj, destDir);
    }

    void shutDown()
    {
        /** @todo Implement shutdown. All running threads should finish before the function returns */
    }
};

ThreadPool* ThreadPool::instance()
{
    static ThreadPoolImpl* p = 0;
    if (!p)
    {
        p = new ThreadPoolImpl();
        if (p)
        {
            Dll* dll = Dll::instance();
            if (dll)
            {
                dll->registerProcessEndCallBack(
                        reinterpret_cast<PdllCallBack>(&ThreadPoolImpl::kill), p);
            }
        }
    }

    return p;
}

// vim: set et ts=4 ai :

