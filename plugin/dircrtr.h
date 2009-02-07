/**
 * @file dircrtr.h
 * The file contains declaration of DirectoryCreator class.
 *
 * $Id: dircrtr.h 71 2008-09-30 15:57:42Z eleskine $
 */
#ifndef __KARBAZOL_DRGNDROP_2_0__DIRCRTR_H__
#define __KARBAZOL_DRGNDROP_2_0__DIRCRTR_H__

#include "mystring.h"

/**
 * Class to create directories
 */
class DirectoryCreator
{
private:
    MyStringW _root;
    bool createDirectoryRecursive(const wchar_t* subdir) const;
public:
    DirectoryCreator(): _root(){}
    DirectoryCreator(const wchar_t* dir);
    ~DirectoryCreator(){}
    /**
     * Tryes to create specified directory.
     * If the directory already exists the function fails.
     */
    bool createDirectory(const wchar_t* subdir, unsigned int attr) const;

    /**
     * Checks whether specified subdir exists.
     */
    bool checkDirectory(const wchar_t* subdir) const;

    /**
     * Creates specified directory.
     * If the directory already exists the functions does nothing
     * but succeeds.
     */
    bool ensureDirectory(const wchar_t* subdir, unsigned int attr) const;

    inline const MyStringW& root() const {return _root;}
};

#endif // __KARBAZOL_DRGNDROP_2_0__DIRCRTR_H__
// vim: set et ts=4 ai :

