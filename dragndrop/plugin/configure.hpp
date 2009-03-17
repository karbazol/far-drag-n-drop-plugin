/**
 * @file configure.hpp
 * The file contains declaration of Config class.
 * 
 * $Id$
 */
#ifndef __KARBAZOL_DRAGNDROP_2_0__CONFIGURE_HPP__
#define __KARBAZOL_DRAGNDROP_2_0__CONFIGURE_HPP__

#include <windows.h>

/**
 * The class is used to manipulate with configuration settings
 */
class Config
{
private:
    // Registry manipulation stuff
    static const wchar_t* _allowDropName;
    static const wchar_t* _checkKeyName;
    static const wchar_t* _shellCopyName;
    static const wchar_t* _showMenuName;
    static const wchar_t* _useShellObjectName;
private:
    DWORD _checkKey;
    bool _allowDrop;
    bool _shellCopy;
    bool _showMenu;
    bool _useShellObject;
    // Disable copy constructor
    Config(const Config&);
    Config();
    ~Config(){}
    static void kill(Config* p);
public:
    static Config* instance();
    inline DWORD checkKey() const {return _checkKey;}
    void checkKey(DWORD value);
    inline bool allowDrop() const {return _allowDrop;}
    void allowDrop(bool value);
    inline bool shellCopy() const {return _shellCopy;}
    void shellCopy(bool value);
    inline bool showMenu() const {return _showMenu;}
    void showMenu(bool value);
    inline bool useShellObject() const {return _useShellObject;}
    void useShellObject(bool value);
};

int doConfigure(int);

#endif // __KARBAZOL_DRAGNDROP_2_0__CONFIGURE_HPP__
// vim: set et ts=4 ai :

