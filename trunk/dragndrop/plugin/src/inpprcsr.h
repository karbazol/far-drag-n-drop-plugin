/**
 * @file inpprcsr.h
 * Contains declaration of InputProcessor class
 *
 * $Id$
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__INPPRCSR_H__
#define __KARBAZOL_DRAGNDROP_2_0__INPPRCSR_H__

#include <windows.h>

/**
* Singleton. Determines whether input state is suiteble to start dragging.
*/
class InputProcessor
{
private:
    InputProcessor();
    InputProcessor(const InputProcessor&); // Disable copy-constructor
    ~InputProcessor();
    static void killInputProcessor(InputProcessor* p);
public:
    /**
     * Returns pointer to the InputProcessors's instance.
     */
    static InputProcessor* instance();

    static BOOL ProcessConsoleInput(HANDLE console, PINPUT_RECORD buffer,
        DWORD buffLength, LPDWORD readCount,
        bool isUnicode, bool removeFromInput);

    bool readFromSystem(HANDLE h, bool waitForInput);
    /**
     * Appends input records to the internal buffer.
     * The function expects input to be in the UTF-16 encoding
     */
    void appendBuffer(PINPUT_RECORD buffer, DWORD records);
    DWORD getBuffer(PINPUT_RECORD buffer, DWORD records, bool unicode, bool remove);
    bool checkThread();
    inline size_t size(){return _userSize;}
    void clear();
    bool isMouseWithinRect (int left, int top, int right, int bottom) const;
private:
    DWORD readBuffer(PINPUT_RECORD buffer, DWORD records, bool unicode);
    bool enterGrabbing(INPUT_RECORD& record);
    bool leaveGrabbing(INPUT_RECORD& record);
    void growBuff(DWORD newCap);

    void processBuffer();
    /**
     * Returns true if record is not MOUSE_EVENT or KEYBOARD_EVENT
     */
    bool checkEvent(INPUT_RECORD& record);
    /**
     * Returns true if there is a screen-grabing.
     */
    bool checkGrabbing(INPUT_RECORD& record);
    bool checkMouseAndShowPopupMenu(INPUT_RECORD& record);
    bool checkKeyBoard(INPUT_RECORD& record);
    bool checkMouseButtons(INPUT_RECORD& record);
private:
    int _left;
    int _right;
    bool _grabbing;
    DWORD _thread;
    PINPUT_RECORD _buffer;
    size_t _buffSize;
    size_t _userSize;
    size_t _buffCapacity;

    MOUSE_EVENT_RECORD _lastMouse;
    KEY_EVENT_RECORD _lastKey;

};

#endif // __KARBAZOL_DRAGNDROP_2_0__INPPRCSR_H__
// vim: set et ts=4 :
