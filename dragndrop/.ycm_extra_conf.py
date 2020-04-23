import os
import ycm_core

MYDIR = os.path.dirname(__file__)

FLAGS = [
#    '-Wall',
#    '-Wextra',
#    '-Werror',
#    '-Wno-long-long',
#    '-Wno-variadic-macros',
#    '-fexceptions',
#    '-ferror-limit=10000',
#    '-DNDEBUG',
#    '-std=c++17',
#    '-xc',
#    '-isystem/usr/include/'
    ]

SOURCE_EXTENSIONS = ['.cpp', '.cxx', '.cc', '.c']

def getIncludeDirs():
    return [os.path.join(MYDIR, x).replace("\\", "/") for x in ('common/include',
            'dll/include', '../farheaders')]

def Settings(**kwargs):
    return {
        'flags': FLAGS + ['-I' + x for x in getIncludeDirs()],
        'do_cache': True
        }
