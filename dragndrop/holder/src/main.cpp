
#include <utils.h>

#include "holder.h"

int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    InitDbgTrace();

    int res = -1;

    Holder holder;
    if (holder.initialize())
    {
        res = holder.run();
    }

#ifndef _DEBUG
    FreeDbgThreadBuff();
    FreeDbgTrace();
#endif
    return res;
}

// vim: set et ts=4 ai :

