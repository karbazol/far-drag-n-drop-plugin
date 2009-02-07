/**
 * @file: The file contains declaration for ThreadPool class.
 *
 * $Id$
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__THRDPOOL_H__
#define __KARBAZOL_DRAGNDROP_2_0__THRDPOOL_H__

#include "fmtprcsr.h"

/**
 * Singleton. Used to keep and track worker threads used to perform
 * asynchronous drag and drop operations.
 */
class ThreadPool
{
private:
    ThreadPool(){}
    ~ThreadPool(){}
    /**
     * This static routine is registered with Dll singleton to
     * destroy ThreadPool singleton on exit.
     */
    static void kill(ThreadPool*);
public:
    /**
     * Returns pointer to the singleton.
     */
    static ThreadPool* instance();

    /**
     * Creates new worker thread and adds it to the thread pool.
     * The thread will process droped data.
     *
     * @param[in] obj points to IDataObject with dropped data.
     * @param[in] destDir specifies a directory where to put the data.
     * @return S_OK on success or the error code on failure.
     */
    HRESULT newThread(IDataObject* obj, const wchar_t* destDir);

    /**
     * Shuts the thread pool down.
     */
    void shutDown();
};

#endif // __KARBAZOL_DRAGNDROP_2_0__THRDPOOL_H__

