#include <windows.h>
#include <common/utils.h>

#include "hookfuncs.h"
#include "modules.hpp"
#include "inpprcsr.h"
#include "dragging.h"

BOOL WINAPI MyReadConsoleInputA(HANDLE h, PINPUT_RECORD b, DWORD l, LPDWORD r)
{
    return InputProcessor::ProcessConsoleInput(h, b, l, r, false, true);
}

BOOL WINAPI MyReadConsoleInputW(HANDLE h, PINPUT_RECORD b, DWORD l, LPDWORD r)
{
    return InputProcessor::ProcessConsoleInput(h, b, l, r, true, true);
}

BOOL WINAPI MyPeekConsoleInputA(HANDLE h, PINPUT_RECORD b, DWORD l, LPDWORD r)
{
    return InputProcessor::ProcessConsoleInput(h, b, l, r, false, false);
}

BOOL WINAPI MyPeekConsoleInputW(HANDLE h, PINPUT_RECORD b, DWORD l, LPDWORD r)
{
    return InputProcessor::ProcessConsoleInput(h, b, l, r, true, false);
}

BOOL WINAPI MyGetNumberOfConsoleInputEvents(
  HANDLE hConsoleInput,
  LPDWORD lpcNumberOfEvents
)
{
    Dragging* dragging = Dragging::instance();
    InputProcessor* inputProcessor = InputProcessor::instance();
    if (!dragging || !dragging->isReadyForDragging() || !inputProcessor)
    {
        return GetNumberOfConsoleInputEvents(hConsoleInput, lpcNumberOfEvents);
    }

    if (IsBadWritePtr(lpcNumberOfEvents, sizeof(*lpcNumberOfEvents)))
    {
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }

    if (!inputProcessor->readFromSystem(hConsoleInput, false))
    {
        return FALSE;
    }

    *lpcNumberOfEvents = static_cast<DWORD>(inputProcessor->size());
    return TRUE;
}

BOOL WINAPI MyWriteConsoleInputA(HANDLE h, const INPUT_RECORD *b, DWORD l, LPDWORD lw)
{
    return WriteConsoleInputA(h, b, l, lw);
}

BOOL WINAPI MyWriteConsoleInputW(HANDLE h, const INPUT_RECORD *b, DWORD l, LPDWORD lw)
{
    return WriteConsoleInputW(h, b, l, lw);
}

BOOL WINAPI MyFlushConsoleInputBuffer(HANDLE h)
{
    Dragging* dragging = Dragging::instance();
    if (dragging && dragging->isReadyForDragging())
    {
        InputProcessor::instance()->clear();
    }
    return FlushConsoleInputBuffer(h);
}

PatchInfo patches[] =
{
    {0, MyReadConsoleInputA,                "ReadConsoleInputA"             },
    {0, MyReadConsoleInputW,                "ReadConsoleInputW"             },
    {0, MyPeekConsoleInputA,                "PeekConsoleInputA"             },
    {0, MyPeekConsoleInputW,                "PeekConsoleInputW"             },
    {0, MyGetNumberOfConsoleInputEvents,    "GetNumberOfConsoleInputEvents" },
    {0, MyWriteConsoleInputA,               "WriteConsoleInputA"            },
    {0, MyWriteConsoleInputW,               "WriteConsoleInputW"            },
    {0, MyFlushConsoleInputBuffer,          "FlushConsoleInputBuffer"       },
};

static void initPatches()
{
    if (patches->func)
        return;

    HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
    size_t i;
    for (i = 0; i < LENGTH(patches); i++)
    {
        patches[i].func = GetProcAddress(hKernel32, patches[i].name);
    }
}

bool patchImports()
{
    initPatches();

    return patchModuleImports(GetModuleHandle(0), patches, LENGTH(patches));
}

void restoreImports()
{
    restoreModuleImports(GetModuleHandle(0), patches, LENGTH(patches));
}

// vim: set et ts=4 ai :

