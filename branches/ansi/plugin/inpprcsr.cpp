/**
 * @file inpprcsr.cpp
 * The file contains implementation of InputProcessor class.
 *
 * $Id: inpprcsr.cpp 71 2008-09-30 15:57:42Z eleskine $
 */

#include <windows.h>
#include "utils.h"
#include "inpprcsr.h"
#include "dll.h"
#include "dragging.h"
#include "mainthrd.h"
#include "configure.hpp"

InputProcessor::InputProcessor(): _left(false), _right(false),
_grabbing(false), _buffer(0), _buffSize(0), _userSize(0),
_buffCapacity(0)
{
    _thread = GetCurrentThreadId();
}

InputProcessor::~InputProcessor()
{
    free(_buffer);
}

/**
 * returns a singleton instance of InputProcessor
 */
InputProcessor* InputProcessor::instance()
{
    static InputProcessor* pInst = {0};
    if (!pInst)
    {
        pInst = new InputProcessor();
        Dll::instance()->registerProcessEndCallBack(reinterpret_cast<PdllCallBack>(killInputProcessor), pInst);
    }

    return pInst;
}

/**
 * destroys the instance of InputProcessor
 */
void InputProcessor::killInputProcessor(InputProcessor* p)
{
    if (p)
        delete p;
}

/**
 * clears the state of InputProcessor
 */
void InputProcessor::clear()
{
    _buffSize = 0;
    _userSize = 0;
}

/**
 * Makes the entire buffer to grow to hold atleast newCap count of event records.
 */
void InputProcessor::growBuff(DWORD newCap)
{
    if (newCap > _buffCapacity)
    {
        _buffCapacity = newCap;
        _buffer = (PINPUT_RECORD)realloc(_buffer, _buffCapacity * sizeof(*_buffer));
    }
}

/**
 * Appends 'records' count of records specified by the 'buffer' to the internal
 * records buffer.
 */
void InputProcessor::appendBuffer(PINPUT_RECORD buffer, DWORD records)
{
    if (!records)
        return;

    size_t newSize = _buffSize + records;

    if (newSize > _buffCapacity)
        growBuff(newSize+255);

    memmove(_buffer + _buffSize, buffer, records*sizeof(*_buffer));

    _buffSize = newSize;

    processBuffer();
}

DWORD InputProcessor::readBuffer(PINPUT_RECORD buffer, DWORD records, bool unicode)
{
    DWORD res = min(records, _userSize);
    memmove(buffer, _buffer, res*sizeof(*buffer));
    
    if (!unicode)
    {
        DWORD cp = GetConsoleCP();

        for (DWORD i = 0; i < res; i++)
        {
            buffer[i] = _buffer[i];
            
            if (buffer[i].EventType == KEY_EVENT)
            {
                WideCharToMultiByte(cp, 0, &buffer[i].Event.KeyEvent.uChar.UnicodeChar, 1,
                    &buffer[i].Event.KeyEvent.uChar.AsciiChar, 2, NULL, NULL);
            }
        }
    }

    return res;
}

DWORD InputProcessor::getBuffer(PINPUT_RECORD buffer, DWORD records, bool unicode, bool remove)
{
    DWORD res = readBuffer(buffer, records, unicode);

    if (remove)
    {
        memmove(_buffer, _buffer + res, (_buffSize - res)*sizeof(*_buffer));
        _buffSize -= res;
        _userSize -= res;
    }

    return res;
}

/**
 * Mask of NOT pressed NumLock, ScrollLock and Caps lock
 */
#define NOT_X_LOCK ~(NUMLOCK_ON|SCROLLLOCK_ON|CAPSLOCK_ON|ENHANCED_KEY)
bool InputProcessor::enterGrabbing(INPUT_RECORD& record)
{
    if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown
        && ((record.Event.KeyEvent.dwControlKeyState & NOT_X_LOCK) == LEFT_ALT_PRESSED
        ||(record.Event.KeyEvent.dwControlKeyState & NOT_X_LOCK) == RIGHT_ALT_PRESSED)
        && (record.Event.KeyEvent.wVirtualKeyCode == VK_INSERT || record.Event.KeyEvent.wVirtualKeyCode == VK_NUMPAD0))
    {
        TRACE("Entering grabbing\n");
        _grabbing = true;
    }
    return _grabbing;
}

bool InputProcessor::leaveGrabbing(INPUT_RECORD& record)
{
    if ((record.EventType == KEY_EVENT
        && record.Event.KeyEvent.bKeyDown
        && (((record.Event.KeyEvent.dwControlKeyState & NOT_X_LOCK) == LEFT_CTRL_PRESSED
        || (record.Event.KeyEvent.dwControlKeyState & NOT_X_LOCK) == RIGHT_CTRL_PRESSED)
        && (record.Event.KeyEvent.wVirtualKeyCode == VK_INSERT || record.Event.KeyEvent.wVirtualKeyCode == VK_NUMPAD0))
        ||record.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)||
        (record.EventType == MOUSE_EVENT &&
        record.Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED))
    {
        TRACE("About to leave grabbing\n");
        _grabbing = false;
    }
    return true;
}

bool InputProcessor::checkThread()
{
    return _thread != GetCurrentThreadId();
}

bool InputProcessor::checkMouse(INPUT_RECORD& record)
{
    if (record.EventType == MOUSE_EVENT)
    {
        ASSERT(!_grabbing);

        if (record.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
        {
            _left++;
        }
        else
        {
            _left = 0;
        }

        if (record.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED)
        {
            _right++;
            if (!_left)
            {
                record.Event.MouseEvent.dwButtonState &= ~RIGHTMOST_BUTTON_PRESSED;
                record.Event.MouseEvent.dwButtonState |= FROM_LEFT_1ST_BUTTON_PRESSED;
            }
        }
        else
        {
            _right = 0;
        }
    }
    return false;
}

bool InputProcessor::checkGrabbing(INPUT_RECORD& record)
{
    if (!_grabbing)
        return enterGrabbing(record);
    else
        return leaveGrabbing(record);
}

bool InputProcessor::checkKeyBoard(INPUT_RECORD& record)
{
    if (record.EventType != MOUSE_EVENT)
        return true;
    if ((record.Event.MouseEvent.dwControlKeyState & Config::instance()->checkKey()) 
            != Config::instance()->checkKey())
        return true;
    return false;
}

bool InputProcessor::checkMouseButtons(INPUT_RECORD& record)
{
    if (record.EventType != MOUSE_EVENT)
        return true;
    if (record.Event.MouseEvent.dwEventFlags & DOUBLE_CLICK)
    {
        TRACE("Received double click event\n");
        _left = 0;
        _right = 0;
        return true;
    }

    if (_left > 1 || _right > 1)
        return false;

    return true;
}

bool InputProcessor::checkEvent(INPUT_RECORD& record)
{
    if (record.EventType != MOUSE_EVENT && record.EventType != KEY_EVENT)
        return true;
    return false;
}

bool InputProcessor::isMouseWithinRect(int left, int top, int right, int buttom) const
{
    ASSERT(_buffSize && _buffer->EventType == MOUSE_EVENT);
    int x = _buffer->Event.MouseEvent.dwMousePosition.X;
    int y = _buffer->Event.MouseEvent.dwMousePosition.Y;

    return x < right && x > left && y < buttom && y > top;
}

bool InputProcessor::readFromSystem(HANDLE h, bool waitForInput)
{
    DWORD systemCount;

    if (waitForInput && !_buffSize)
    {
        DWORD res = WaitForSingleObject(h, INFINITE);

        if (res != WAIT_OBJECT_0)
            return false;
    }

    if (!GetNumberOfConsoleInputEvents(h, &systemCount))
        return false;

    if (systemCount)
    {
        size_t newSize = _buffSize + systemCount;

        growBuff(newSize);

        if (!_buffer)
            FatalAppExit(0, L"Memory allocation failed");

        if (!ReadConsoleInputW(h, _buffer + _buffSize, systemCount, &systemCount))
            return false;

        _buffSize += systemCount;
    }

    return true;
}

void InputProcessor::processBuffer()
{
    if (_userSize)
        return;
    for (;_userSize < _buffSize; _userSize++)
    {
        if (checkEvent(_buffer[_userSize])||
            checkGrabbing(_buffer[_userSize])||
            checkMouse(_buffer[_userSize])||
            checkKeyBoard(_buffer[_userSize])
            )
        {
            continue;
        }    
        
        if (checkMouseButtons(_buffer[_userSize]))
        {
            _userSize++;
            return;
        }

        if (Dragging::instance()->start())
        {
            // Cut buffer
            clear();
            return;
        }
    }
}

static BOOL ProcessDirectInput(HANDLE console, PINPUT_RECORD buffer,
        DWORD buffLength, LPDWORD readCount,
        bool isUnicode, bool removeFromInput)
{
    if (removeFromInput)
    {
        if (isUnicode)
            return ReadConsoleInputW(console, buffer, buffLength, readCount);
        else
            return ReadConsoleInputA(console, buffer, buffLength, readCount);
    }
    else
    {
        if (isUnicode)
            return PeekConsoleInputW(console, buffer, buffLength, readCount);
        else
            return PeekConsoleInputA(console, buffer, buffLength, readCount);
    }
}

/**
 * The main loop entry function.
 */
BOOL InputProcessor::ProcessConsoleInput(HANDLE console, PINPUT_RECORD buffer,
        DWORD buffLength, LPDWORD readCount,
        bool isUnicode, bool removeFromInput)
{
    // Process messages in asynchronous queue
    MainThread::instance()->processMessage(false);

    if (!Dragging::instance()->isReadyForDragging())
    {
        return ProcessDirectInput(console, buffer, buffLength, readCount, isUnicode, removeFromInput);
    }

    if (IsBadWritePtr(readCount, sizeof(*readCount))||
        IsBadWritePtr(buffer, buffLength*sizeof(*buffer)))
    {
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }

    InputProcessor* processor = InputProcessor::instance();

    if (!processor->readFromSystem(console, removeFromInput))
        return FALSE;

    // Analyze the buffer contents and if it contains
    // appropriate keyboard/mouse state start dragging
    processor->processBuffer();

    if (processor->size())
    {
        *readCount = processor->getBuffer(buffer, buffLength, isUnicode, removeFromInput);
    }
    else
    {
        *readCount = 0;
    }

    return TRUE;
}

// vim: set et ts=4 :
