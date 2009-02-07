/**
 * @file configure.hpp
 * The file contains declaration of Config class.
 * 
 * $Id: configure.hpp 75 2008-10-02 17:51:35Z eleskine $
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
private:
    DWORD _checkKey;
    bool _allowDrop;
    bool _shellCopy;
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
};

#endif // __KARBAZOL_DRAGNDROP_2_0__CONFIGURE_HPP__
// vim: set et ts=4 ai :

