// $Id$

/**
 * @file cpydlg.h
 * Contains declaration of CopyDialog class
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__CPYDLG_H__
#define __KARBAZOL_DRAGNDROP_2_0__CPYDLG_H__

#include "fardlg.h"
#include "mystring.h"
#include "filelist.h"
#include "filecopy.h"


/**
 * Implements Far-based copy progress dialog.
 */
class CopyDialog : public FarDialog, public FileListNotify, public FileCopier::FileCopyNotify
{
private:
#pragma pack(push, 1)
    struct CopyDialogItems
    {
        InitDialogItem frame;
        InitDialogItem label0;
        InitDialogItem lblSrcFile;
        InitDialogItem label1;
        InitDialogItem lblDestFile;
        InitDialogItem progressCurrent;
        InitDialogItem lblTotalSize;
        InitDialogItem progressTotal;
        InitDialogItem label3;
        InitDialogItem lblFilesProcessed;
        InitDialogItem label4;
        InitDialogItem lblTimeInfo;
    };
#pragma pack(pop)
    static CopyDialogItems copyDialogItemsTemplate;
    wchar_t szFilesProcessed[41];
    CopyDialogItems _items;
    __int64 _totalProcessedSize;
    __int64 _totalSize;
    __int64 _currentProcessedSize;
    __int64 _currentSize;
    /*MyStringW _srcFile;
    MyStringW _destFile;*/
    volatile long _filesProcessed;
    volatile long _filesToProcess;
    volatile long _fileListProcessed;
    LARGE_INTEGER _timeStart;
    int _speed;
    void updateTotalSize();
    void updateFilesProcessed();
    void updatePercents();
    void updateProgressBar(int value, int controlId);
    void updateTimesAndSpeed();
    void calcSpeed();
private: /* FileListNotify implementation */
    bool onNextEntry(const int reason, const FileListEntry& e);
    bool onAllProcessed();
private: /* FileCopier::FileCopyNotify */
    bool onFileExists(const wchar_t* src, const wchar_t* dest);
    bool onFileStep(const __int64& step);
    bool onFileError(const wchar_t* src, const wchar_t* dest, DWORD errorNumber);
protected:
    InitDialogItem* items();
    int itemsCount();
    int right();
    int bottom();
    DWORD flags();
protected:
    bool onInit();
    bool onClose(int id);
public:
    CopyDialog();
    ~CopyDialog(){}

    bool appendFile(const __int64& size, bool lastOne);
    bool nextFile(const wchar_t* src, const wchar_t* dest, const __int64& size);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__CPYDLG_H__

