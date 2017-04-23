/**
 * @file filelist.cpp
 * Contains implementation of FileList class.
 *
 */

#include <utils.h>

#include "filelist.h"

/**
 * @brief Container for file list entries.
 *
 * Utility class used by FileList objects to hold
 * FileListEntry objects.
 */
class FileListEntryContainer
{
private:
    FileListEntryContainer* _next;
    FileListEntry _entry;
    ~FileListEntryContainer()
    {
    }
public:
    FileListEntryContainer(const FileListEntry& e, FileListEntryContainer* insertAfter):
        _next(0), _entry(e)
    {
        if (insertAfter)
            insertAfter->_next = this;
    }
    FileListEntryContainer* deleteIt()
    {
        FileListEntryContainer* next;

        if (this)
        {
            next = _next;
            delete this;
        }
        else
        {
            next = 0;
        }

        return next;
    }

    void deleteAll()
    {
        FileListEntryContainer* p = this;

        while (p)
        {
            p = p->deleteIt();
        }
    }

    FileListEntry& entry()
    {
        return _entry;
    }

    const FileListEntry& entry() const
    {
        return _entry;
    }
};

FileListEntry::FileListEntry(): _srcpath(), _subpath(0), _type(InvalidType)
{
}

FileListEntry::FileListEntry(const FileListEntry& e):
    _srcpath(e._srcpath), _subpath(e._subpath), _data(e._data), _type(e._type)
{
}

FileListEntry& FileListEntry::operator=(const FileListEntry& e)
{
    _srcpath = e._srcpath;
    _subpath = e._subpath;
    _data = e._data;
    _type = e._type;

    return *this;
}

FileListEntry::FileListEntry(const MyStringW& cmnDir, const wchar_t* subpath,
            const WIN32_FIND_DATA& data, EType type): _srcpath(cmnDir), _data(data), _type(type)
{
    if (!_srcpath)
    {
        _srcpath = subpath;
        _subpath = 0;
    }
    else
    {
        _srcpath /= subpath;
        _subpath = lstrlen(cmnDir);

        if (_srcpath[_subpath] == L'\\') _subpath++;
    }
}

const wchar_t* FileListEntry::subpath() const
{
    if (!_srcpath)
        return L"";

    return _srcpath + _subpath;
}

DWORD FileList::runMe(FileList* This)
{
    return This->createTheList(This->_files);
}

FileList::FileList(const wchar_t* files, FileListNotify* fn): RefCounted(), _notify(fn),
    _files(files), _entriesGuard(), _head(0), _tail(0)
{
    if (_notify)
    {
        _notify->addRef();
    }
    _haveEntries = CreateEvent(NULL, FALSE, FALSE, NULL);
    _stopThread = CreateEvent(NULL, FALSE, FALSE, NULL);

#if 1
    DWORD dwThrdId;
    _threadEnd = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&runMe),
            this, 0, &dwThrdId);
#else
    runMe(this);
#endif
}

FileList::~FileList()
{
    if (_notify)
    {
        _notify->release();
        _notify = 0;
    }
#if 1
    SignalObjectAndWait(_stopThread, _threadEnd, INFINITE, FALSE);
    CloseHandle(_threadEnd);
#endif
    CloseHandle(_haveEntries);
    CloseHandle(_stopThread);

    _head->deleteAll();
}

unsigned int FileList::createTheList(const wchar_t* files)
{
    if (files)
    {
        for(;*files;files += lstrlen(files)+1)
        {
            if (!processEntry(files))
            {
                return GetLastError();
            }
        }
    }

    if (_notify && !_notify->onAllProcessed())
        return GetLastError();

    return 0;
}

bool FileList::appendEntry(const FileListEntry& e)
{
    if (WaitForSingleObject(_stopThread, 0) == WAIT_OBJECT_0)
        return false;

    {
        LOCKIT(_entriesGuard);

        _tail = new FileListEntryContainer(e, _tail);
        if (!_head)
            _head = _tail;

        if (WaitForSingleObject(_haveEntries, 0) != WAIT_OBJECT_0)
            SetEvent(_haveEntries);

        if (_notify && !_notify->onNextEntry(0, e))
            return false;
    }

    return true;
}

bool FileList::processEntry(const wchar_t* file)
{
    MyStringW entry(file);

    normalizePath(entry);

    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFile(entry, &fd);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    FindClose(h);

    MyStringW prefix;
    MyStringW basename;

    splitAndAbsPath(entry, prefix, basename);

    if (FILE_ATTRIBUTE_DIRECTORY == (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        FileListEntry e(prefix, basename, fd, FileListEntry::EnterDirectory);
        if (!appendEntry(e))
            return false;
        bool ret = processDir(prefix, e);
        FileListEntry e2(prefix, basename, fd, FileListEntry::LeaveDirectory);
        if (!appendEntry(e2))
            return false;
        return ret;
    }
    else
    {
        FileListEntry e(prefix, basename, fd, FileListEntry::File);
        return appendEntry(e);
    }
}

bool FileList::processDir(const wchar_t* prefix, const FileListEntry& e)
{
    MyStringW mask = e.srcpath();
    mask /= L"*";

    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFile(mask, &fd);
    if (h == INVALID_HANDLE_VALUE)
        return false;

    bool res = true;
    do
    {
        if (lstrcmp(L"..", fd.cFileName) && lstrcmp(L".", fd.cFileName))
        {
            MyStringW subpath = e.subpath();
            subpath /= fd.cFileName;

            if (FILE_ATTRIBUTE_DIRECTORY ==
                (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                FileListEntry entry(prefix, subpath, fd, FileListEntry::EnterDirectory);
                if (!appendEntry(entry))
                {
                    res = false;
                    break;
                }
                res = processDir(prefix, entry);
                FileListEntry e2(prefix, subpath, fd, FileListEntry::LeaveDirectory);
                if (!appendEntry(e2))
                    res = false;
                if (!res)
                    break;
            }
            else
            {
                FileListEntry entry(prefix, subpath, fd, FileListEntry::File);
                if (!appendEntry(entry))
                {
                    res = false;
                    break;
                }
            }
        }
    }
    while (FindNextFile(h, &fd));
    FindClose(h);

    return res;
}

bool FileList::next(FileListEntry& e)
{
    HANDLE events[] = {_threadEnd, _haveEntries};
    WaitForMultipleObjects(LENGTH(events), events, FALSE, INFINITE);

    LOCKIT(_entriesGuard);

    if (_head)
    {
        e = _head->entry();
        _head = _head->deleteIt();
        if (!_head)
        {
            _tail = _head;
            ResetEvent(_haveEntries);
        }
        return true;
    }

    return false;
}

// vim: set et ts=4 ai :

