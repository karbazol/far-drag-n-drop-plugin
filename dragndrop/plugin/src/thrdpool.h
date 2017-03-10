/**
 * @file: thrdpool.h
 * The file contains declaration for ThreadPool class.
 *
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__THRDPOOL_H__
#define __KARBAZOL_DRAGNDROP_2_0__THRDPOOL_H__

#include <windows.h>

/**
 * Singleton. Used to keep and track worker threads used to perform
 * asynchronous drag and drop operations.
 */
struct ThreadPool
{
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
    virtual HRESULT newThread(IDataObject* obj, const wchar_t* destDir) = 0;

    /**
     * Shuts the thread pool down.
     */
    virtual void shutDown() = 0;
};

#endif // __KARBAZOL_DRAGNDROP_2_0__THRDPOOL_H__

