/**
 * @file mainthrd.h
 * The file contains declaration of MainThread class
 *
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__MAINTHRD_H_
#define __KARBAZOL_DRAGNDROP_2_0__MAINTHRD_H_

#include <windows.h>
#include <ddlock.h>
#include <growarry.h>
#include <irefcounted.hpp>
#include <mystring.h>

// Main thread message identifiers

/**
 * setDragging(bool value)\n
 * param0 - boolean value
 */
#define MTM_SETDRAGGING 1

/** Window thread started */
#define MTM_WINTHRSTARTED 2

/**
 *Get current directory of panel under cursor\n
 * param0 - POINT* of screen coordinates\n
 * param1 - pointer to MyStringW structure to receive dir\n
 * returns true on success otherwise false\n
 */
#define MTM_GETDIRFROMPT 3

/**
 * Send callable object
 *  param0 - pointer to a Callable object instance.
 *  after the object is called it will be release'ed therefore it has to be addRef'ed before posting the message
 */
#define MTM_CALLTHEOBJECT 4

/**
 * Singleton. Represents a main thread of Far.
 */
class MainThread
{
public:
    /**
     * Interface of callable object.
     */
    class Callable: public IRefCounted
    {
    public:
        virtual void* call() = 0;
    };
private:
    DWORD _threadId;
    CriticalSection _sendGuard;
    CriticalSection _postGuard;
    HANDLE _eventMessage;
    HANDLE _eventProcessed;
private:
    /**
     * Message to handle
     */
    struct Message
    {
        unsigned int _msg;
        void* _param0;
        void* _param1;
        void* _param2;
        void* _param3;
    } _msg;
    GrowOnlyArray<Message> _posted;
    void* _result;
private:
    MainThread();
    ~MainThread();
    static void kill(MainThread* p);
    void* handleMessage(unsigned int msg, void* param0, void* param1, void* param2, void* param3);
    void* sendMessage(unsigned int msg, void* param0=NULL, void* param1=NULL, void* param2=NULL, void* param3=NULL);
    void postMessage(unsigned int msg, void* param0=NULL, void* param1=NULL, void* param2=NULL, void* param3=NULL);
    void processPostedMessages();
    bool popPostedMessage(Message& m);
private:
    // Message handlers
    void onSetDragging(bool value);
    bool onGetDirFromScreenPoint(POINT&pt, MyStringW& dir);
    void* onCallIt(void* (*function)(void*), void* param);
public:
    static MainThread* instance();
    bool isMainThread();
    // returns last processed message id
    unsigned int processMessage(bool wait, void** result=NULL);
    void* waitForMessage(unsigned int msg);
    inline void setDragging(bool value){sendMessage(MTM_SETDRAGGING, (void*)value);}
    inline void winThreadStarted(){sendMessage(MTM_WINTHRSTARTED);}
    inline bool getDirFromScreenPoint(POINT& pt, MyStringW& dir)
    {
        return sendMessage(MTM_GETDIRFROMPT, &pt, &dir)?true:false;
    }
    inline void* callIt(void* (*function)(void*), void* param){return sendMessage(MTM_CALLTHEOBJECT, function, param);}
    inline void callItAsync(void* (*function)(void*), void* param){postMessage(MTM_CALLTHEOBJECT, function, param);}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__MAINTHRD_H_
// vim: set et ts=4 ai :

