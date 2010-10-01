######################################
### Microsoft compiler configurations default
######################################

import sys
from parts.config import *
import SCons.Script

def map_default_version(env):
    return env['MSVC_VERSION']

config=configuration(map_default_version)

config.VersionRange("6.0-7.1",
                    append=ConfigValues(
                        LINKFLAGS=['/opt:nowin98'],
                        LIBS=['libcmt']
                        )
                    )

config.VersionRange("*",
                    append=ConfigValues(
                        CPPDEFINES=['NDEBUG'],
                        CCFLAGS=[
                            '/MT',  # Use multi-threaded static crt
                            '/Oxs', # Optimize them all
                            ],
                        LINKFLAGS=[
                            '/NODEFAULTLIB'
                            ]
                        )
                    )
    



