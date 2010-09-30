######################################
### Microsoft compiler configurations default
######################################

import sys
from parts.config import *
import SCons.Script

def map_default_version(env):
    return env['MSVC_VERSION']

config=configuration(map_default_version)

config.VersionRange("*",
                    append=ConfigValues(
                        CPPDEFINES=['_DEBUG'],
                        CCFLAGS=[
                            '/MTd', # Use multi-threaded static CRT
                            '/Z7',  # Use msvc 7.1 compatible debug info
                            '/Od',  # No optimization
                            ],
                        LINKFLAGS=[
                            '/INCREMENTAL:NO', '/DEBUG', '/OPT:REF'
                            ]
                        )
                    )
    



