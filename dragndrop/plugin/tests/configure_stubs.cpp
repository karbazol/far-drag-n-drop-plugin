#include <list>
#include "../src/fardlg.h"
#include <dll/dll.h>

#include "../../common/src/ddlock.cpp"

FarDialog::FarDialog()
{
}

FarDialog::~FarDialog()
{
}

LONG_PTR FarDialog::handle(int msg, int param1, LONG_PTR param2)
{
    return 0;
}

int FarDialog::left()
{
    return 0;
}

int FarDialog::top()
{
    return 0;
}

int FarDialog::right()
{
	return 0;
}

int FarDialog::bottom()
{
	return 0;
}

const wchar_t* FarDialog::help()
{
    return L"";
}

InitDialogItem* FarDialog::items()
{
	return 0;
}

DWORD FarDialog::flags()
{
	return 0;
}

bool FarDialog::onInit()
{
	return 0;
}

bool FarDialog::onClose(int closeId)
{
	return 0;
}

int FarDialog::itemsCount()
{
	return 0;
}


int FarDialog::show(bool modal)
{
	return 0;
}

int FarDialog::hide()
{
	return 0;
}


LONG_PTR FarDialog::sendMessage(int msg, int param1, LONG_PTR param2)
{
	return 0;
}

void FarDialog::postMessage(int msg, int param1, LONG_PTR param2)
{
	return;
}


// Operations with the dialog controls
bool FarDialog::enable(int id)
{
	return 0;
}

bool FarDialog::disable(int id)
{
	return 0;
}

int FarDialog::switchCheckBox(int id, int state)
{
	return 0;
}

int FarDialog::checkState(int id)
{
	return 0;
}

bool FarDialog::checked(int id)
{
	return 0;
}

bool FarWriteRegistry(const wchar_t* name, const DWORD value)
{
	return false;
}

DWORD FarReadRegistry(const wchar_t* name, DWORD defaultValue)
{
	return defaultValue;
}

struct CallBackInfo_
{
    PdllCallBack callBack;
    void* data;
};

static std::list<CallBackInfo_> callbacks;

Dll::~Dll()
{
    std::list<CallBackInfo_>::const_iterator p;

    for (p = callbacks.begin(); p != callbacks.end(); p++)
    {
        p->callBack(p->data);
    }
}

void Dll::registerProcessEndCallBack(PdllCallBack func, void* data)
{
    CallBackInfo_ p = {func, data};

    callbacks.push_back(p);
}

Dll* Dll::instance()
{
    static Dll dll;

    return &dll;
}

// vim: set et ts=4 sw=4 ai :

