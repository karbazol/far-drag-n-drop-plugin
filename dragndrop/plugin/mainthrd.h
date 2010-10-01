/**
 * @file mainthrd.h
 * The file contains declaration of MainThread class
 *
 * $Id$
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__MAINTHRD_H_
#define __KARBAZOL_DRAGNDROP_2_0__MAINTHRD_H_

#include <windows.h>
#include "growarry.h"
#include "mystring.h"
#include "ddlock.h"

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
 * Send message to a far dialog\n
 * param0 - HANDLE of the dialog\n
 * param1 - int Msg to be sent\n
 * param2 - int Param1 of the message\n
 * param3 - int Param2 of the message\n
 */
#define MTM_SENDDLGMSG 4

/**
 * Send callable object
 *  param0 - pointer to a Callable object instance
 */
#define MTM_CALLTHEOBJECT 5

/**
 * Singleton. Represents a main thread of Far.
 */
class MainThread
{
public:
    /**
     * Interface of callable object.
     */
    class Callable
    {
    public:
        virtual ~Callable(){}
        virtual void* call() = 0;
        virtual void free();
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
    void* onCallIt(Callable* p);
    LONG_PTR onSendDlgMessage(void* msg);
public:
    static MainThread* instance();
    // returns last processed message id
    unsigned int processMessage(bool wait, void** result=NULL);
    void* waitForMessage(unsigned int msg);
    inline void setDragging(bool value){sendMessage(MTM_SETDRAGGING, (void*)value);}
    inline void winThreadStarted(){sendMessage(MTM_WINTHRSTARTED);}
    inline bool getDirFromScreenPoint(POINT& pt, MyStringW& dir)
    {
        return sendMessage(MTM_GETDIRFROMPT, &pt, &dir)?true:false;
    }
    inline LONG_PTR sendDlgMessage(void* msg)
    {
        return reinterpret_cast<LONG_PTR>(sendMessage(MTM_SENDDLGMSG, msg));
    }
    inline void postDlgMessage()
    {
        postMessage(MTM_SENDDLGMSG, 0);
    }
    inline void* callIt(Callable* p){return sendMessage(MTM_CALLTHEOBJECT, p);}
    inline void callItAsync(Callable* p){postMessage(MTM_CALLTHEOBJECT, p);}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__MAINTHRD_H_
// vim: set et ts=4 ai :

