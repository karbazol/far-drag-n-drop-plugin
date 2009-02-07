/**
 * @file The file contains declaration of Config class.
 * 
 * $Id$
 */
#ifndef __KARBAZOL_DRAGNDROP_2_0__CONFIGURE_HPP__
#define __KARBAZOL_DRAGNDROP_2_0__CONFIGURE_HPP__

#include <windows.h>

class Config
{
private:
    // Registry manipulation stuff
    static const char* _allowDropName;
    static const char* _checkKeyName;
    static const char* _shellCopyName;
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

