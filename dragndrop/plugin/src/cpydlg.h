// $Id: cpydlg.h 81 2011-11-07 08:50:02Z Karbazol $

/**
 * @file cpydlg.h
 * Contains declaration of CopyDialog class
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__CPYDLG_H__
#define __KARBAZOL_DRAGNDROP_2_0__CPYDLG_H__

#include "fardlg.h"
#include <dll/mystring.h>
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
    CopyDialogItems _items;
    int64_t _totalProcessedSize;
    int64_t _totalSize;
    int64_t _currentProcessedSize;
    int64_t _currentSize;
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
    bool onFileStep(const int64_t& step);
    bool onFileError(const wchar_t* src, const wchar_t* dest, DWORD errorNumber);
protected:
    const GUID& Id() const;
    InitDialogItem* items();
    size_t itemsCount();
    int right();
    int bottom();
    DWORD flags();
protected:
    bool onInit();
    bool onClose(intptr_t id);
public:
    CopyDialog();
    ~CopyDialog(){}

    bool appendFile(const int64_t& size, bool lastOne);
    bool nextFile(const wchar_t* src, const wchar_t* dest, const int64_t& size);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__CPYDLG_H__

