/**
 * @file filecopy.h
 * Contains declaration of FileCopier class.
 *
 * $Id$
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__FILECOPY_H__
#define __KARBAZOL_DRAGNDROP_2_0__FILECOPY_H__

#include <windows.h>
#include "mystring.h"

/**
 * Instances of this class perform copying of a single file.
 */
class FileCopier
{
public:
    /**
     * Interface used to notify about file copying progress
     */
    struct FileCopyNotify
    {
        virtual ~FileCopyNotify(){};
        /**
         * @brief What to do if the file allready exists
         *
         * The function is called by FileCopier when it trys to
         * copy file to already existent file.
         * Return true to overrwrite existing file
         */
        virtual bool onFileExists(const wchar_t* src, const wchar_t* dest) = 0;
        virtual bool onFileStep(const __int64& step) = 0;
        /**
         * @brief What to do in case of error
         *
         * Return true if the FileCopier object should try again
         */
        virtual bool onFileError(const wchar_t* src, const wchar_t* dest,
                DWORD errorNumber) = 0;
    };
private:
    MyStringW _src;
    MyStringW _dest;
    FileCopyNotify* _notify;
    __int64 _copied;
    bool _result;
    static DWORD CALLBACK winCallBack(LARGE_INTEGER totalSize, LARGE_INTEGER transferred,
            LARGE_INTEGER streamSize, LARGE_INTEGER streamBytesTransferred,
            DWORD streamNumber, DWORD dwCallBackReason, HANDLE hSourcefile,
            HANDLE hDestinationFile, FileCopier* This);
    bool doCopy();
public:
    /**
     * @param[in] src points to the name of source file.
     * @param[in] dest points to the name of destenation file.
     * @param[in] p is optional and points to instance of FileCopyNotify to receive
     *           notifications during file copy/move operation.
     */
    FileCopier(const wchar_t* src, const wchar_t* dest, FileCopyNotify* p = NULL);
    ~FileCopier(){}

    /**
     * This method allows to get result of copy/move operation performed.
     * To determine error code use GetLastError() function.
     *
     * @return true on success and false on failure.
     */
    inline bool result() const {return _result;}
};

#endif // __KARBAZOL_DRAGNDROP_2_0__FILECOPY_H__
