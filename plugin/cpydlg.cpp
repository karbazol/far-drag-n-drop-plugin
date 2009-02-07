// $Id: cpydlg.cpp 26 2008-04-20 18:48:32Z eleskine $

#include "cpydlg.h"
#include "ddlng.h"
#include "utils.h"

CopyDialog::CopyDialogItems CopyDialog::copyDialogItemsTemplate =
{
        /*  0 */{DI_DOUBLEBOX,3,1,46,13,0,0,0,0,(char*)MCopy},
        /*  1 */{DI_TEXT,5, 2,44,0,0,0,0,0,(char*)MCopyingTheFile},
        /*  2 */{DI_TEXT,5, 3,44,0,0,0,0,0,""},
        /*  3 */{DI_TEXT,5, 4,44,0,0,0,0,0,(char*)MCopyingTo},
        /*  4 */{DI_TEXT,5, 5,44,0,0,0,0,0,""},
        /*  5 */{DI_TEXT,5, 6,44,0,0,0,0,0,"같같같같같같같같같같같같같같같같같같같같"},
        /*  6 */{DI_TEXT,5, 7,44,0,0,0,0,0,"Total size:"},
        /*  7 */{DI_TEXT,5, 8,44,0,0,0,0,0,"같같같같같같같같같같같같같같같같같같같같"},
        /*  8 */{DI_TEXT,5, 9,44,0,0,0,0,0,"컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴"},
        /*  9 */{DI_TEXT,5,10,44,0,0,0,0,0,""},
        /* 10 */{DI_TEXT,5,11,44,0,0,0,0,0,"컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴"},
        /* 11 */{DI_TEXT,5,12,44,0,0,0,0,0,"Time: %.2d:%.2d:%.2d Left: %.2d:%.2d:%.2d %6dKb/s"}
};

#define getItemId(structItem) ((int)&((CopyDialogItems*)0)->structItem / sizeof(InitDialogItem))
#define NANOSECPERSEC 10000000

CopyDialog::CopyDialog(): FarDialog(), _items(copyDialogItemsTemplate),
    _totalProcessedSize(0), _totalSize(0), _currentProcessedSize(0),
    _currentSize(0), _srcFile(), _destFile(), _filesToProcess(0),
    _filesProcessed(-1), _fileListProcessed(false), _speed(0)
{
    wsprintfA(szFilesProcessed, GetMsg(MFilesProcessed), 0, 0);

    _items.lblFilesProcessed.Data = szFilesProcessed;

    _timeStart.QuadPart = 0;
}

bool CopyDialog::onInit()
{
    updateTimesAndSpeed();
    updateFilesProcessed();
    updatePercents();
    updateTimesAndSpeed();
    return true;
}

bool CopyDialog::onClose(int id)
{
    id;
    return true;
}

static char* sizeToString(char* buff, const __int64& value)
{
    wsprintfA(buff, "%I64d", value);

    size_t len = lstrlenA(buff);
    size_t mod = len % 3;
    size_t div = len / 3;

    while (div)
    {
        if (mod)
        {
            memmove(buff + mod + 1, buff + mod, len - mod + 2);
            buff[mod] = ',';
            len++;
            mod++;
        }

        mod += 3;
        div--;
    }

    return buff;
}

static char* centerAndFill(char* buff, size_t size /*of buff including NULL treminator*/,
        char filler)
{
    size_t len = lstrlenA(buff);
    size_t pos = (size - 1 - len) / 2;

    memmove(buff + pos, buff, len);

    size_t i;
    size_t pos2 = pos + len;
    for (i = 0; i < pos; i++)
    {
        buff[i] = buff[pos2++] = filler;
    }

    if (pos2 < size-1)
        buff[pos2++] = filler;

    buff[pos2] = '\0';

    return buff;
}

void CopyDialog::updateTotalSize()
{
    char totalSizeValue[41];
    char totalSizeString[41];

    wsprintfA(totalSizeString, GetMsg(MTotalSize), sizeToString(totalSizeValue, _totalSize));

    postMessage(DM_SETTEXTPTR, getItemId(lblTotalSize), 
            (long)centerAndFill(totalSizeString, LENGTH(totalSizeString), '\xc4'));
}

void CopyDialog::updateFilesProcessed()
{
    char filesProcessed[41];

    int processed;
    if (_filesProcessed < 0)
        processed = 0;
    else
        processed = _filesProcessed;

    wsprintfA(filesProcessed, GetMsg(MFilesProcessed), processed, _filesToProcess);

    postMessage(DM_SETTEXTPTR, getItemId(lblFilesProcessed), (long)filesProcessed);
}

static int calcPercents(const __int64& value, const __int64& base, int len)
{
    if (!value)
        return 0;

    if (!base)
        return len;

    return static_cast<int>(value * len / base);
}

void CopyDialog::updatePercents()
{
    int percents;
    if (_fileListProcessed)
    {
        percents = calcPercents(_totalProcessedSize, _totalSize, 40);
        updateProgressBar(percents, getItemId(progressTotal));
    }

    percents = calcPercents(_currentProcessedSize, _currentSize, 40);
    updateProgressBar(percents, getItemId(progressCurrent));
}

void CopyDialog::updateProgressBar(int value, int controlId)
{
    char stringValue[41];

    int i = value - 1;
    int j = value;
    while (i >= 0 || j < LENGTH(stringValue)-1)
    {
        if (i >=0)
            stringValue[i--] = '\xdb';
        if (j < LENGTH(stringValue)-1)
            stringValue[j++] = '\xb0';
    }
    stringValue[j] = '\0';

    postMessage(DM_SETTEXTPTR, controlId, (long)stringValue);
}

void CopyDialog::updateTimesAndSpeed()
{
    if (!_timeStart.QuadPart)
        GetSystemTimeAsFileTime(reinterpret_cast<LPFILETIME>(&_timeStart));
    LARGE_INTEGER spent;
    GetSystemTimeAsFileTime(reinterpret_cast<LPFILETIME>(&spent));
    spent.QuadPart -= _timeStart.QuadPart;

    LARGE_INTEGER left = {0};

    if (_speed)
    {
        left.QuadPart = (_totalSize - _totalProcessedSize) / (_speed) * NANOSECPERSEC;
    }

    SYSTEMTIME spentTime, leftTime;

    FileTimeToSystemTime(reinterpret_cast<LPFILETIME>(&spent), &spentTime);
    FileTimeToSystemTime(reinterpret_cast<LPFILETIME>(&left), &leftTime);

    char timeString[MAX_PATH];

    wsprintfA(timeString, 
            GetMsg(MFileCopyingTimes), // "Time: %.2d:%.2d:%.2d Left: %.2d:%.2d:%.2d %6dKb/s"
            spentTime.wHour, spentTime.wMinute, spentTime.wSecond,
            leftTime.wHour, leftTime.wMinute, leftTime.wSecond,
            _speed >> 10);

    postMessage(DM_SETTEXTPTR, getItemId(lblTimeInfo), (long)timeString); 
}

void CopyDialog::calcSpeed()
{
    LARGE_INTEGER now;
    GetSystemTimeAsFileTime(reinterpret_cast<LPFILETIME>(&now));

    __int64 delta = now.QuadPart - _timeStart.QuadPart;
    if (delta)
    {
        _speed = static_cast<int>(_totalProcessedSize * NANOSECPERSEC / delta);
    }
    else
    {
        _speed = 1;
    }
}

bool CopyDialog::appendFile(const __int64& size, bool lastOne)
{
    if (!running())
        return false;
    if (lastOne)
    {
        _fileListProcessed = true;
    }
    else
    {
        _totalSize += size;
        _filesToProcess++;
        if (hwnd())
        {
            updateTotalSize();
            updateFilesProcessed();
        }
    }

    return true;
}

bool CopyDialog::nextFile(const wchar_t* src, const wchar_t* dest,
        const __int64& size)
{
    if (running())
    {
        _filesProcessed++;
        _srcFile = w2a(src, CP_OEMCP);
        _destFile = w2a(dest, CP_OEMCP);

        _totalProcessedSize += (_currentSize - _currentProcessedSize);
        _currentProcessedSize = 0;
        _currentSize = size;

        TruncPathStr(_srcFile, 40);
        TruncPathStr(_destFile, 40);

        postMessage(DM_SETTEXTPTR, getItemId(lblSrcFile), (long)(char*)_srcFile);
        postMessage(DM_SETTEXTPTR, getItemId(lblDestFile), (long)(char*)_destFile);

        updateFilesProcessed();
        updatePercents();

        return true;
    }

    return false;
}

bool CopyDialog::step(const __int64& step)
{
    if (running())
    {
        _currentProcessedSize += step;
        _totalProcessedSize += step;
        
        calcSpeed();

        updateTimesAndSpeed();
        updatePercents();
        
        return true;
    }
    return false;
}

InitDialogItem* CopyDialog::items()
{
    return &_items.frame;
}

int CopyDialog::itemsCount()
{
    return sizeof(CopyDialogItems)/sizeof(InitDialogItem);
}

int CopyDialog::right()
{
    return _items.frame.X2+4;
}

int CopyDialog::bottom()
{
    return _items.frame.Y2+2;
}

// vim: set et ts=4 ai :

