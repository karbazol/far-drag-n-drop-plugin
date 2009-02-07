// $Id$

#ifndef __KARBAZOL_DRAGNDROP_2_0__CPYDLG_H__
#define __KARBAZOL_DRAGNDROP_2_0__CPYDLG_H__

#include "fardlg.h"
#include "mystring.h"

class CopyDialog : public FarDialog
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
        InitDialogItem progressTotal;
        InitDialogItem lblTotalSize;
        InitDialogItem progressCurrent;
        InitDialogItem label3;
        InitDialogItem lblFilesProcessed;
        InitDialogItem label4;
        InitDialogItem lblTimeInfo;
    };
#pragma pack(pop)
    static CopyDialogItems copyDialogItemsTemplate;
    char szFilesProcessed[41];
    CopyDialogItems _items;
    __int64 _totalProcessedSize;
    __int64 _totalSize;
    __int64 _currentProcessedSize;
    __int64 _currentSize;
    MyStringA _srcFile;
    MyStringA _destFile;
    int _filesProcessed;
    int _filesToProcess;
    bool _fileListProcessed;
    LARGE_INTEGER _timeStart;
    int _speed;
    void updateTotalSize();
    void updateFilesProcessed();
    void updatePercents();
    void updateProgressBar(int value, int controlId);
    void updateTimesAndSpeed();
    void calcSpeed();
protected:
    InitDialogItem* items();
    int itemsCount();
    int right();
    int bottom();
protected:
    bool onInit();
    bool onClose(int id);
public:
    CopyDialog();
    ~CopyDialog(){}

    bool appendFile(const __int64& size, bool lastOne);
    bool nextFile(const wchar_t* src, const wchar_t* dest, const __int64& size);
    bool step(const __int64& step);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__CPYDLG_H__

